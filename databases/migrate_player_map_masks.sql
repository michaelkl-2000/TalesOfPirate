-- =====================================================================
-- Migration: map_mask (single-row, 5 fixed columns) -> player_map_masks
-- Date: 2026-05-14
-- Idempotent: safe to run multiple times.
-- Cleanup (DROP TABLE map_mask, DROP COLUMN character.map_mask) -- only
-- after C++ rollout is verified; see cleanup_legacy_mapmask.sql.
-- =====================================================================

USE [GameDB];
GO

SET ANSI_NULLS ON;
SET QUOTED_IDENTIFIER ON;
GO

-- ---------------------------------------------------------------------
-- 1) Create new table if missing.
-- ---------------------------------------------------------------------
IF NOT EXISTS (SELECT 1 FROM sys.tables WHERE name = 'player_map_masks' AND schema_id = SCHEMA_ID('dbo'))
BEGIN
    CREATE TABLE [dbo].[player_map_masks] (
        [id]         [int]          IDENTITY(1,1) NOT NULL,
        [atorID]     [int]          NOT NULL,
        [map_name]   [varchar](32)  NOT NULL,
        [mask_data]  [varchar](max) NOT NULL,
        [updated_at] [datetime]     NOT NULL CONSTRAINT [DF_player_map_masks_updated_at] DEFAULT (GETDATE()),
        CONSTRAINT [PK_player_map_masks] PRIMARY KEY CLUSTERED ([id] ASC),
        CONSTRAINT [UQ_player_map_masks_atorID_map_name] UNIQUE NONCLUSTERED ([atorID] ASC, [map_name] ASC)
    );

    CREATE NONCLUSTERED INDEX [IX_player_map_masks_atorID]
        ON [dbo].[player_map_masks] ([atorID] ASC);

    PRINT 'Table player_map_masks created.';
END
ELSE
BEGIN
    PRINT 'Table player_map_masks already exists -- skipping CREATE.';
END
GO

-- FK on character(atorID). Создаём отдельно от CREATE TABLE, чтобы при повторных запусках
-- скрипт чинил недостающий FK (например, если ранее упал на ошибке имени колонки).
-- character.atorID — это PK в этой схеме (название историческое, "actor/account ID" обманчиво).
IF EXISTS (SELECT 1 FROM sys.tables WHERE name = 'character' AND schema_id = SCHEMA_ID('dbo'))
   AND EXISTS (SELECT 1 FROM sys.tables WHERE name = 'player_map_masks' AND schema_id = SCHEMA_ID('dbo'))
   AND NOT EXISTS (SELECT 1 FROM sys.foreign_keys WHERE name = 'FK_player_map_masks_character')
BEGIN
    ALTER TABLE [dbo].[player_map_masks]
        ADD CONSTRAINT [FK_player_map_masks_character]
            FOREIGN KEY ([atorID]) REFERENCES [dbo].[character]([atorID])
            ON DELETE CASCADE;
    PRINT 'FK FK_player_map_masks_character created.';
END
ELSE
BEGIN
    PRINT 'FK FK_player_map_masks_character already exists or prerequisites missing -- skipping.';
END
GO

-- ---------------------------------------------------------------------
-- 2) Migrate data from legacy map_mask.contentN -> player_map_masks rows.
--    The legacy mapping was:
--       content1 = garner
--       content2 = magicsea
--       content3 = darkblue
--       content4 = winterland
--       content5 = (unused, no name binding)
--    Empty/whitespace-only content cells are skipped.
--    Insert is guarded by NOT EXISTS so the script is idempotent.
-- ---------------------------------------------------------------------
IF EXISTS (SELECT 1 FROM sys.tables WHERE name = 'map_mask' AND schema_id = SCHEMA_ID('dbo'))
BEGIN
    ;WITH legacy AS (
        SELECT atorID, 'garner'     AS map_name, RTRIM(content1) AS mask_data
            FROM [dbo].[map_mask] WHERE LEN(RTRIM(ISNULL(content1, ''))) > 0
        UNION ALL
        SELECT atorID, 'magicsea',   RTRIM(content2)
            FROM [dbo].[map_mask] WHERE LEN(RTRIM(ISNULL(content2, ''))) > 0
        UNION ALL
        SELECT atorID, 'darkblue',   RTRIM(content3)
            FROM [dbo].[map_mask] WHERE LEN(RTRIM(ISNULL(content3, ''))) > 0
        UNION ALL
        SELECT atorID, 'winterland', RTRIM(content4)
            FROM [dbo].[map_mask] WHERE LEN(RTRIM(ISNULL(content4, ''))) > 0
    )
    INSERT INTO [dbo].[player_map_masks] (atorID, map_name, mask_data)
    SELECT l.atorID, l.map_name, l.mask_data
    FROM legacy l
    WHERE NOT EXISTS (
        SELECT 1 FROM [dbo].[player_map_masks] p
        WHERE p.atorID = l.atorID AND p.map_name = l.map_name
    );

    PRINT CONCAT('Inserted rows in this run: ', @@ROWCOUNT);
END
ELSE
BEGIN
    PRINT 'map_mask table not found -- nothing to migrate (clean install?)';
END
GO

-- ---------------------------------------------------------------------
-- 3) Verification report.
--    new_rows      = current count in player_map_masks
--    legacy_rows   = current count in map_mask (single row per player)
--    expected_rows = sum of non-empty content cells across map_mask
-- ---------------------------------------------------------------------
DECLARE @new_rows      INT = (SELECT COUNT(*) FROM [dbo].[player_map_masks]);
DECLARE @legacy_rows   INT = NULL;
DECLARE @expected_rows INT = NULL;

IF EXISTS (SELECT 1 FROM sys.tables WHERE name = 'map_mask' AND schema_id = SCHEMA_ID('dbo'))
BEGIN
    SELECT @legacy_rows = COUNT(*) FROM [dbo].[map_mask];

    SELECT @expected_rows = SUM(
              CASE WHEN LEN(RTRIM(ISNULL(content1, ''))) > 0 THEN 1 ELSE 0 END
            + CASE WHEN LEN(RTRIM(ISNULL(content2, ''))) > 0 THEN 1 ELSE 0 END
            + CASE WHEN LEN(RTRIM(ISNULL(content3, ''))) > 0 THEN 1 ELSE 0 END
            + CASE WHEN LEN(RTRIM(ISNULL(content4, ''))) > 0 THEN 1 ELSE 0 END
        )
    FROM [dbo].[map_mask];
END

SELECT
    @new_rows      AS new_rows,
    @legacy_rows   AS legacy_rows,
    @expected_rows AS expected_rows,
    CASE
        WHEN @expected_rows IS NULL THEN 'no legacy table'
        WHEN @new_rows >= @expected_rows THEN 'OK'
        ELSE 'MISSING ROWS -- investigate'
    END AS status;
GO

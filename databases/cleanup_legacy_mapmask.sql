-- =====================================================================
-- Cleanup: drop legacy map_mask table and character.map_mask column.
-- RUN ONLY AFTER:
--   1. migrate_player_map_masks.sql executed successfully;
--   2. C++ GameServer rebuilt and running on the new schema;
--   3. .NET Admin/EF Core no longer references Character.MapMask;
--   4. You have a backup of map_mask in case of rollback.
-- =====================================================================

USE [GameDB];
GO

-- Drop legacy column character.map_mask (orphan int FK on map_mask.id after refactor).
-- Note: character.atorID is the PK (legacy naming), not the column we drop here.
-- На колонке может висеть DEFAULT/CHECK constraint и/или индексы — удаляем их сначала.
IF EXISTS (SELECT 1 FROM sys.columns WHERE name = 'map_mask' AND object_id = OBJECT_ID('dbo.character'))
BEGIN
    DECLARE @sql NVARCHAR(MAX);

    -- 1) Сбросить DEFAULT/CHECK constraint'ы на колонке map_mask
    DECLARE constraint_cursor CURSOR FOR
        SELECT 'ALTER TABLE [dbo].[character] DROP CONSTRAINT [' + dc.name + ']'
        FROM sys.default_constraints dc
        JOIN sys.columns c ON c.object_id = dc.parent_object_id AND c.column_id = dc.parent_column_id
        WHERE dc.parent_object_id = OBJECT_ID('dbo.character') AND c.name = 'map_mask'
        UNION ALL
        SELECT 'ALTER TABLE [dbo].[character] DROP CONSTRAINT [' + cc.name + ']'
        FROM sys.check_constraints cc
        JOIN sys.columns c ON c.object_id = cc.parent_object_id AND c.column_id = cc.parent_column_id
        WHERE cc.parent_object_id = OBJECT_ID('dbo.character') AND c.name = 'map_mask';

    OPEN constraint_cursor;
    FETCH NEXT FROM constraint_cursor INTO @sql;
    WHILE @@FETCH_STATUS = 0
    BEGIN
        EXEC sp_executesql @sql;
        PRINT 'Dropped constraint: ' + @sql;
        FETCH NEXT FROM constraint_cursor INTO @sql;
    END
    CLOSE constraint_cursor;
    DEALLOCATE constraint_cursor;

    -- 2) Сбросить индексы, использующие колонку map_mask
    DECLARE index_cursor CURSOR FOR
        SELECT DISTINCT 'DROP INDEX [' + i.name + '] ON [dbo].[character]'
        FROM sys.indexes i
        JOIN sys.index_columns ic ON ic.object_id = i.object_id AND ic.index_id = i.index_id
        JOIN sys.columns c ON c.object_id = ic.object_id AND c.column_id = ic.column_id
        WHERE i.object_id = OBJECT_ID('dbo.character')
          AND c.name = 'map_mask'
          AND i.is_primary_key = 0
          AND i.is_unique_constraint = 0;

    OPEN index_cursor;
    FETCH NEXT FROM index_cursor INTO @sql;
    WHILE @@FETCH_STATUS = 0
    BEGIN
        EXEC sp_executesql @sql;
        PRINT 'Dropped index: ' + @sql;
        FETCH NEXT FROM index_cursor INTO @sql;
    END
    CLOSE index_cursor;
    DEALLOCATE index_cursor;

    -- 3) Сама колонка
    ALTER TABLE [dbo].[character] DROP COLUMN [map_mask];
    PRINT 'Dropped column character.map_mask.';
END
ELSE
BEGIN
    PRINT 'Column character.map_mask not present -- skipping.';
END
GO

-- Drop legacy table map_mask
IF EXISTS (SELECT 1 FROM sys.tables WHERE name = 'map_mask' AND schema_id = SCHEMA_ID('dbo'))
BEGIN
    DROP TABLE [dbo].[map_mask];
    PRINT 'Dropped table map_mask.';
END
ELSE
BEGIN
    PRINT 'Table map_mask not present -- skipping.';
END
GO

#pragma once

#include <string>
class CPicture
{
public:
	CPicture(std::string name);
	CPicture(const CPicture &rPic);
	~CPicture();
	
	CPicture& operator=(const CPicture &rPic);

	std::string GetName() { return m_strName; }
	void SetName(std::string name) { m_strName = name; }

	char GetID() { return m_nID; }
	void SetID(char id) { m_nID = id; }

	std::uint32_t GetSize() { return m_size; }
	
	char GetImgByte(std::uint32_t index);

	bool LoadImg();

private:
	void SetSize(std::uint32_t size) { m_size = size; }

	std::string m_strName; //
	char m_nID;
	char *_img;
	std::uint32_t m_size;
	std::uint32_t m_start;
};

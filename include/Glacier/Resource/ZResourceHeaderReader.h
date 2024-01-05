#pragma once

#include "SResourceHeaderHeader.h"
#include "ZRuntimeResourceID.h"

class ZResourceHeaderReader
{
public:
	ZResourceHeaderReader(const SResourceHeaderHeader* headerHeader, unsigned char* pReferencesChunk);
	unsigned int GetNumResourceIdentifiers() const;
	ZRuntimeResourceID GetResourceIdentifier(unsigned int lResourceIdentifierIndex) const;

private:
	const SResourceHeaderHeader* m_HeaderHeader;
	unsigned char* m_pReferencesChunk;
};

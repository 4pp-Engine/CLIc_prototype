/*  CLIc - version 0.1 - Copyright 2020 Stéphane Rigaud, Robert Haase,
*   Institut Pasteur Paris, Max Planck Institute for Molecular Cell Biology and Genetics Dresden
*
*   CLIc is part of the clEsperanto project http://clesperanto.net 
*
*   This file is subject to the terms and conditions defined in
*   file 'LICENSE.txt', which is part of this source code package.
*/



#include "cleInt.h"

#include <string>

namespace cle
{

Int::Int(int v)
{
    value = v;
}

int& Int::GetData()
{
    return value;
}

std::string Int::GetObjectType() const
{
    return this->ObjectTypeToString(O);
}

std::string Int::ToString() const
{
    std::string str = this->GetObjectType() + "(int) = " + std::to_string(value);
    return str;
}

bool Int::IsObject(std::string str) const
{
    return this->GetObjectType() == str;
}

size_t Int::GetBitSize() const
{
    return sizeof(int);
}

} // namespace cle


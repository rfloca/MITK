#ifndef MITKSTRINGPROPERTY_H_HEADER_INCLUDED_C1C02491
#define MITKSTRINGPROPERTY_H_HEADER_INCLUDED_C1C02491

#include "mitkCommon.h"
#include "mitkBaseProperty.h"
#include <string>

namespace mitk {

//##ModelId=3E3FDF05028B
//##Documentation
//## @brief Property for strings
//## @ingroup DataTree
class StringProperty : public BaseProperty
{
protected:
    //##ModelId=3E3FDF21017D
    std::string m_String;

public:
    mitkClassMacro(StringProperty, BaseProperty);

    //##ModelId=3E3FF04F005F
    StringProperty( const char* string );
    StringProperty( std::string  s );
    
    itkGetStringMacro(String);
    itkSetStringMacro(String);

    //##ModelId=3E3FF04F00E1
    virtual bool operator==(const BaseProperty& property ) const;
    virtual std::string GetValueAsString() const;
};

} // namespace mitk

#endif /* MITKSTRINGPROPERTY_H_HEADER_INCLUDED_C1C02491 */

#include "exampleHeaderFile.hpp"

/**
This function returns the integer that results from casting a double variable to int (expect trucation of any values past the decimal place).  For arbitrary reasons, you must not pass this function a negative value.
@param inputNumberToConvert: The double number to convert
@return: The converted value (after trucation to integer)

@throws This function can throw exceptions
*/
int thisFunctionConvertsToInt(double inputNumberToConvert)
{
if(inputNumberToConvert < 0.0)
{
throw std::logic_error("thisFunctionConvertsToInt does not like negative numbers\n");
}

return inputNumberToConvert;
} 

/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkExprTkFunctionParser.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/**
 * @class   vtkExprTkFunctionParser
 * @brief   Parse and evaluate a mathematical expression
 *
 * vtkExprTkFunctionParser is a wrapper class of the ExprTK library that takes
 * in a mathematical expression as a char string, parses it, and evaluates it
 * at the specified values of the variables in the input string.
 *
 * The detailed documentation of the supported functionality is described in
 * https://github.com/ArashPartow/exprtk. In addition to the documented
 * functionality, the following vector operations have been implemented:
 * 1) cross(v1, v2), cross product of two vectors,
 * 2) mag(v), magnitude of a vector,
 * 3) norm(v), the normalized version of a vector.
 *
 * @par Thanks:
 * Arash Partow (arash@partow.net) for implementing the ExprTk library.
 */

#ifndef vtkExprTkFunctionParser_h
#define vtkExprTkFunctionParser_h

#include "vtkCommonMiscModule.h" // For export macro
#include "vtkObject.h"
#include "vtkTuple.h" // needed for vtkTuple
#include <string>     // needed for string.
#include <vector>     // needed for vector

// the value that is returned as a result if there is an error
#define VTK_PARSER_ERROR_RESULT VTK_FLOAT_MAX

// forward declarations for ExprTk tools
struct vtkExprTkTools;

class VTKCOMMONMISC_EXPORT vtkExprTkFunctionParser : public vtkObject
{
public:
  static vtkExprTkFunctionParser* New();
  vtkTypeMacro(vtkExprTkFunctionParser, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /**
   * Return parser's MTime
   */
  vtkMTimeType GetMTime() override;

  ///@{
  /**
   * Set/Get input string to evaluate.
   */
  void SetFunction(const char* function);
  vtkGetStringMacro(Function);
  ///@}

  /**
   * Check whether the result is a scalar result.  If it isn't, then
   * either the result is a vector or an error has occurred.
   */
  int IsScalarResult();

  /**
   * Check whether the result is a vector result.  If it isn't, then
   * either the result is scalar or an error has occurred.
   */
  int IsVectorResult();

  /**
   * Get a scalar result from evaluating the input function.
   */
  double GetScalarResult();

  ///@{
  /**
   * Get a vector result from evaluating the input function.
   */
  double* GetVectorResult() VTK_SIZEHINT(3);
  void GetVectorResult(double result[3])
  {
    double* r = this->GetVectorResult();
    result[0] = r[0];
    result[1] = r[1];
    result[2] = r[2];
  }
  ///@}

  ///@{
  /**
   * Set the value of a scalar variable.  If a variable with this name
   * exists, then its value will be set to the new value.  If there is not
   * already a variable with this name, variableName will be added to the
   * list of variables, and its value will be set to the new value. If the
   * variable name is not sanitized, it should be provided in quotes, and
   * a valid unique string will be used as a replacement by the parser.
   *
   * @note A sanitized variable name is accepted by the following regex: ^[a-zA-Z][a-zA-Z_0-9]*.
   */
  void SetScalarVariableValue(const char* variableName, double value);
  void SetScalarVariableValue(int i, double value);
  ///@}

  ///@{
  /**
   * Get the value of a scalar variable.
   */
  double GetScalarVariableValue(const char* variableName);
  double GetScalarVariableValue(int i);
  ///@}

  ///@{
  /**
   * Set the value of a vector variable.  If a variable with this name
   * exists, then its value will be set to the new value.  If there is not
   * already a variable with this name, variableName will be added to the
   * list of variables, and its value will be set to the new value. If the
   * variable name is not sanitized, it should be provided in quotes, and
   * a valid unique string will be used as a replacement by the parser.
   *
   * @note A sanitized variable name is accepted by the following regex: ^[a-zA-Z][a-zA-Z_0-9]*.
   */
  void SetVectorVariableValue(
    const char* variableName, double xValue, double yValue, double zValue);
  void SetVectorVariableValue(const char* variableName, const double values[3])
  {
    this->SetVectorVariableValue(variableName, values[0], values[1], values[2]);
  }
  void SetVectorVariableValue(int i, double xValue, double yValue, double zValue);
  void SetVectorVariableValue(int i, const double values[3])
  {
    this->SetVectorVariableValue(i, values[0], values[1], values[2]);
  }
  ///@}

  ///@{
  /**
   * Get the value of a vector variable.
   */
  double* GetVectorVariableValue(const char* variableName) VTK_SIZEHINT(3);
  void GetVectorVariableValue(const char* variableName, double value[3])
  {
    double* r = this->GetVectorVariableValue(variableName);
    value[0] = r[0];
    value[1] = r[1];
    value[2] = r[2];
  }
  double* GetVectorVariableValue(int i) VTK_SIZEHINT(3);
  void GetVectorVariableValue(int i, double value[3])
  {
    double* r = this->GetVectorVariableValue(i);
    value[0] = r[0];
    value[1] = r[1];
    value[2] = r[2];
  }
  ///@}

  /**
   * Get the number of scalar variables.
   */
  int GetNumberOfScalarVariables()
  {
    return static_cast<int>(this->UsedScalarVariableNames.size());
  }

  /**
   * Get scalar variable index or -1 if not found
   */
  int GetScalarVariableIndex(const char* name);

  /**
   * Get the number of vector variables.
   */
  int GetNumberOfVectorVariables()
  {
    return static_cast<int>(this->UsedVectorVariableNames.size());
  }

  /**
   * Get scalar variable index or -1 if not found
   */
  int GetVectorVariableIndex(const char* name);

  /**
   * Get the ith scalar variable name.
   */
  const char* GetScalarVariableName(int i);

  /**
   * Get the ith vector variable name.
   */
  const char* GetVectorVariableName(int i);

  ///@{
  /**
   * Returns whether a scalar variable is needed for the function evaluation.
   * This is only valid after a successful Parse(). Thus, call GetScalarResult()
   * or IsScalarResult() or similar method before calling this.
   */
  bool GetScalarVariableNeeded(int i);
  bool GetScalarVariableNeeded(const char* variableName);
  ///@}

  ///@{
  /**
   * Returns whether a vector variable is needed for the function evaluation.
   * This is only valid after a successful Parse(). Thus, call GetVectorResult()
   * or IsVectorResult() or similar method before calling this.
   */
  bool GetVectorVariableNeeded(int i);
  bool GetVectorVariableNeeded(const char* variableName);
  ///@}

  /**
   * Remove all the current variables.
   */
  void RemoveAllVariables();

  /**
   * Remove all the scalar variables.
   */
  void RemoveScalarVariables();

  /**
   * Remove all the vector variables.
   */
  void RemoveVectorVariables();

  ///@{
  /**
   * When ReplaceInvalidValues is on, all invalid values (such as
   * sqrt(-2), note that function parser does not handle complex
   * numbers) will be replaced by ReplacementValue. Otherwise an
   * error will be reported
   */
  vtkSetMacro(ReplaceInvalidValues, vtkTypeBool);
  vtkGetMacro(ReplaceInvalidValues, vtkTypeBool);
  vtkBooleanMacro(ReplaceInvalidValues, vtkTypeBool);
  vtkSetMacro(ReplacementValue, double);
  vtkGetMacro(ReplacementValue, double);
  ///@}

  /**
   * Allow the user to force the function to be re-parsed
   */
  void InvalidateFunction();

  /**
   * Sanitize a label/name to remove spaces, delimiters etc.
   * Once the label/name is sanitized is can be accepted by the
   * following regex: ^[a-zA-Z][a-zA-Z_0-9]*.
   *
   * @note taken from vtkSMCoreUtilities
   */
  static std::string SanitizeName(const char* name);

protected:
  vtkExprTkFunctionParser();
  ~vtkExprTkFunctionParser() override;

  /**
   * Parses the given function, returning true on success, false on failure.
   * It has 2 modes (0, 1). The first mode parses the function and uses a return statement
   * to identify the ReturnType. The second mode parses the function and uses a result
   * vector to store the results of the function knowing its return type. Both modes
   * need to be utilized to extract results of a function. The second mode is used because
   * it's a lot faster than the first.
   */
  int Parse(int mode);

  /**
   * Evaluate the function, returning true on success, false on failure.
   */
  bool Evaluate();

  /**
   * Collects meta-data about which variables are needed by the current
   * function. This is called only after a successful call to this->Parse().
   */
  void UpdateNeededVariables();

  vtkSetStringMacro(ParseError);

  char* Function;
  std::string FunctionWithUsedVariableNames;
  std::string ExpressionString;

  // original and used variables names are the same, except if the original
  // ones are not valid.
  std::vector<std::string> OriginalScalarVariableNames;
  std::vector<std::string> UsedScalarVariableNames;
  std::vector<std::string> OriginalVectorVariableNames;
  std::vector<std::string> UsedVectorVariableNames;
  // pointers for scalar and vector variables are used to enforce
  // that their memory address will not change due to a possible
  // resizing of their container (std::vector)
  std::vector<double*> ScalarVariableValues;
  std::vector<vtkTuple<double, 3>*> VectorVariableValues;
  std::vector<bool> ScalarVariableNeeded;
  std::vector<bool> VectorVariableNeeded;

  vtkTimeStamp FunctionMTime;
  vtkTimeStamp ParseMTime;
  vtkTimeStamp VariableMTime;
  vtkTimeStamp EvaluateMTime;

  vtkTypeBool ReplaceInvalidValues;
  double ReplacementValue;

  char* ParseError;

  vtkExprTkTools* ExprTkTools;

  int ResultType;
  vtkTuple<double, 3> Result;

private:
  vtkExprTkFunctionParser(const vtkExprTkFunctionParser&) = delete;
  void operator=(const vtkExprTkFunctionParser&) = delete;
};

#endif

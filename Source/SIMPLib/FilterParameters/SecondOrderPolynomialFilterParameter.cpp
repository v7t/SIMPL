/* ============================================================================
* Copyright (c) 2009-2016 BlueQuartz Software, LLC
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.
*
* Redistributions in binary form must reproduce the above copyright notice, this
* list of conditions and the following disclaimer in the documentation and/or
* other materials provided with the distribution.
*
* Neither the name of BlueQuartz Software, the US Air Force, nor the names of its
* contributors may be used to endorse or promote products derived from this software
* without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* The code contained herein was partially funded by the followig contracts:
*    United States Air Force Prime Contract FA8650-07-D-5800
*    United States Air Force Prime Contract FA8650-10-D-5210
*    United States Prime Contract Navy N00173-07-C-2068
*
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include "SecondOrderPolynomialFilterParameter.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SecondOrderPolynomialFilterParameter::SecondOrderPolynomialFilterParameter()
: FilterParameter()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SecondOrderPolynomialFilterParameter::~SecondOrderPolynomialFilterParameter() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SecondOrderPolynomialFilterParameter::Pointer SecondOrderPolynomialFilterParameter::New(const QString& humanLabel, const QString& propertyName, const Float2ndOrderPoly_t& defaultValue,
                                                                                        Category category, SetterCallbackType setterCallback, GetterCallbackType getterCallback, int groupIndex)
{

  SecondOrderPolynomialFilterParameter::Pointer ptr = SecondOrderPolynomialFilterParameter::New();
  ptr->setHumanLabel(humanLabel);
  ptr->setPropertyName(propertyName);
  QVariant v;
  v.setValue(defaultValue);
  ptr->setDefaultValue(v);
  ptr->setCategory(category);
  ptr->setGroupIndex(groupIndex);
  ptr->setSetterCallback(setterCallback);
  ptr->setGetterCallback(getterCallback);

  return ptr;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString SecondOrderPolynomialFilterParameter::getWidgetType() const
{
  return QString("SecondOrderPolynomialWidget");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SecondOrderPolynomialFilterParameter::readJson(const QJsonObject& json)
{
  QJsonValue jsonValue = json[getPropertyName()];
  if(!jsonValue.isUndefined() && m_SetterCallback)
  {
    QJsonObject obj = jsonValue.toObject();
    Float2ndOrderPoly_t poly;
    poly.readJson(obj);
    m_SetterCallback(poly);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SecondOrderPolynomialFilterParameter::writeJson(QJsonObject& json)
{
  if (m_GetterCallback)
  {
    Float2ndOrderPoly_t poly = m_GetterCallback();
    QJsonObject obj;
    poly.writeJson(obj);
    json[getPropertyName()] = obj;
  }
}

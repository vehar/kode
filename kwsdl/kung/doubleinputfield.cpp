/*
    This file is part of Kung.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <QtXml/QDomElement>
#include <knuminput.h>

#include <schema/simpletype.h>

#include "doubleinputfield.h"

DoubleInputField::DoubleInputField(const QString &name, const QString &typeName,
                                   const XSD::SimpleType *type)
    : SimpleInputField(name, type), mValue(0), mTypeName(typeName)
{
}

void DoubleInputField::setXMLData(const QDomElement &element)
{
    if (mName != element.tagName()) {
        qDebug("DoubleInputField: Wrong dom element passed: expected %s, got %s", qPrintable(mName),
               qPrintable(element.tagName()));
        return;
    }

    setData(element.text());
}

void DoubleInputField::xmlData(QDomDocument &document, QDomElement &parent)
{
    QDomElement element = document.createElement(mName);
    element.setAttribute("xsi:type", "xsd:" + mTypeName);
    QDomText text = document.createTextNode(data());
    element.appendChild(text);

    parent.appendChild(element);
}

void DoubleInputField::setData(const QString &data)
{
    mValue = data.toDouble();
}

QString DoubleInputField::data() const
{
    return QString::number(mValue);
}

QWidget *DoubleInputField::createWidget(QWidget *parent)
{
    mInputWidget = new QDoubleSpinBox(parent);

    if (mType) {
        if (mType->facetType() & XSD::SimpleType::MININC)
            mInputWidget->setMinimum(mType->facetMinimumInclusive());
        if (mType->facetType() & XSD::SimpleType::MINEX)
            mInputWidget->setMinimum(mType->facetMinimumExclusive() + 1);
        if (mType->facetType() & XSD::SimpleType::MAXINC)
            mInputWidget->setMaximum(mType->facetMaximumInclusive());
        if (mType->facetType() & XSD::SimpleType::MAXEX)
            mInputWidget->setMaximum(mType->facetMaximumExclusive() - 1);
    }

    mInputWidget->setValue(mValue);

    connect(mInputWidget, SIGNAL(valueChanged(double)), this, SLOT(inputChanged(double)));

    return mInputWidget;
}

void DoubleInputField::inputChanged(double value)
{
    mValue = value;

    emit modified();
}

#include "doubleinputfield.moc"

/*
    This file is part of KDE.

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

#include <common/messagehandler.h>
#include <common/parsercontext.h>

#include "porttype.h"

using namespace KWSDL;

PortType::PortType() {}

PortType::PortType(const QString &nameSpace) : Element(nameSpace) {}

PortType::~PortType() {}

void PortType::setName(const QString &name)
{
    mName = name;
}

QString PortType::name() const
{
    return mName;
}

void PortType::setOperations(const Operation::List &operations)
{
    mOperations = operations;
}

Operation::List PortType::operations() const
{
    return mOperations;
}

void PortType::loadXML(ParserContext *context, const QDomElement &element)
{
    mName = element.attribute("name");
    if (mName.isEmpty())
        context->messageHandler()->warning("PortType: 'name' required");

    QDomElement child = element.firstChildElement();
    while (!child.isNull()) {
        QName tagName = child.tagName();
        if (tagName.localName() == "operation") {
            Operation operation(nameSpace());
            operation.loadXML(context, child);
            mOperations.append(operation);
        } else {
            context->messageHandler()->warning(
                    QString("PortType: unknown tag %1").arg(child.tagName()));
        }

        child = child.nextSiblingElement();
    }
}

void PortType::saveXML(ParserContext *context, QDomDocument &document, QDomElement &parent) const
{
    QDomElement element = document.createElement("portType");
    parent.appendChild(element);

    if (!mName.isEmpty())
        element.setAttribute("name", mName);
    else
        context->messageHandler()->warning("PortType: 'name' required");

    Operation::List::ConstIterator it(mOperations.begin());
    const Operation::List::ConstIterator endIt(mOperations.end());
    for (; it != endIt; ++it)
        (*it).saveXML(context, document, element);
}

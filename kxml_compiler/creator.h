/*
    This file is part of KDE.

    Copyright (c) 2004-2006 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef CREATOR_H
#define CREATOR_H

#include "schema.h"
#include "classdescription.h"

#include <libkode/code.h>
#include <libkode/printer.h>
#include <libkode/typedef.h>
#include <libkode/file.h>

#include <QDebug>

#include <QFile>
#include <QTextStream>
#include <qdom.h>
#include <QRegExp>
#include <QMap>

#include <iostream>

class Creator
{
public:
    class ClassFlags
    {
    public:
        ClassFlags(const Schema::Element &element);

        bool hasUpdatedTimestamp() const;
        bool hasId() const;

    private:
        bool m_hasUpdatedTimestamp;
        bool m_hasId;
    };

    enum XmlParserType { XmlParserDom, XmlParserDomExternal };

    Creator(const Schema::Document &document, XmlParserType p = XmlParserDom);

    void setVerbose(bool verbose);

    void setUseKde(bool useKde);
    bool useKde() const;

    void setCreateCrudFunctions(bool createCrud);

    /**
     * @brief setCreateWriterFunctions
     * This method can be used to enable/disable the generation of the XML
     * generating codes.
     * @param createWriter
     */
    void setCreateWriterFunctions(bool createWriter = true);

    /**
     * @brief setCreateParserFunctions
     * This method can be used to enable/disable the generation of the XML
     * parsing codes.
     * @param createParser
     */
    void setCreateParserFunctions(bool createParser = true);

    void setLicense(const KODE::License &);

    void setExportDeclaration(const QString &name);

    void setExternalClassPrefix(const QString &);

    bool externalParser() const;
    bool externalWriter() const;

    KODE::File &file();
    void setFilename(const QString &baseName);

    void setParserClass(const KODE::Class &);
    KODE::Class &parserClass();

    const Schema::Document &document() const;

    void create();

    void createCrudFunctions(KODE::Class &c, const QString &type);
    void createProperty(KODE::Class &c, const ClassDescription &, const QString &type,
                        const QString &name);

    ClassDescription createClassDescription(const Schema::Element &element);
    void createClass(const Schema::Element &element);

    void registerListTypedef(const QString &type);

    void createListTypedefs();

    void createFileParser(const Schema::Element &element);

    void setDtd(const QString &dtd);

    void createFileWriter(const Schema::Element &element);

    void printFiles(KODE::Printer &);

    QString errorStream() const;
    QString debugStream() const;

    void setUseQEnums(bool useQEnums);
    bool useQEnums() const;

protected:
    void setExternalClassNames();

    void createElementParser(KODE::Class &c, const Schema::Element &e);

    QString typeName(Schema::Node::Type);

private:
    Schema::Document mDocument;

    XmlParserType mXmlParserType;
    QString mExternalClassPrefix;

    KODE::File mFile;
    KODE::Class mParserClass;
    KODE::Class mWriterClass;
    QStringList mProcessedClasses;
    QStringList mListTypedefs;

    QString mBaseName;
    QString mDtd;
    bool mVerbose;
    bool mUseKde;
    bool mCreateCrudFunctions;
    bool mUseQEnums = false;
    bool mCreateWriterFunctions = true;
    bool mCreateParserFunctions = true;
    QString mExportDeclaration;
};

class ParserCreator
{
public:
    ParserCreator(Creator *);
    virtual ~ParserCreator();

    Creator *creator() const;

    virtual void createFileParser(const Schema::Element &element) = 0;
    virtual void createStringParser(const Schema::Element &element) = 0;
    virtual void createElementParser(KODE::Class &c, const Schema::Element &e) = 0;

private:
    Creator *mCreator;
};

#endif

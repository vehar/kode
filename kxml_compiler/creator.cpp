/*
    This file is part of KDE.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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

#include "creator.h"

#include "namer.h"

#include "parsercreatordom.h"
#include "writercreator.h"

#include <libkode/code.h>
#include <libkode/printer.h>
#include <libkode/typedef.h>
#include <libkode/statemachine.h>
#include <schema/simpletype.h>

/*#include <kaboutdata.h>
#include <kapplication.h>
#include <qDebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kstandarddirs.h>*/

#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <qdom.h>
#include <QRegExp>
#include <QMap>
#include <QList>

#include <iostream>

Creator::ClassFlags::ClassFlags( const Schema::Element &element )
{
  m_hasId = element.hasRelation( "id" );
  m_hasUpdatedTimestamp = element.hasRelation( "updated_at" );
}
    
bool Creator::ClassFlags::hasUpdatedTimestamp() const
{
  return m_hasUpdatedTimestamp;
}

bool Creator::ClassFlags::hasId() const
{
  return m_hasId;
}


Creator::Creator( const Schema::Document &document, XmlParserType p )
  : mDocument( document ), mXmlParserType( p ),
    mVerbose( false ), mUseKde( false ), m_createParseFunctions(true), m_createWriteFunctions(true)
{
  setExternalClassNames();
}

// FIXME: Handle creation options via flags enum.

void Creator::setVerbose( bool verbose ) 
{
  mVerbose = verbose;
}

void Creator::setUseKde( bool useKde )
{
  mUseKde = useKde;
}

bool Creator::useKde() const
{
  return mUseKde;
}

void Creator::setCreateCrudFunctions( bool enabled )
{
  mCreateCrudFunctions = enabled;
}

void Creator::setLicense( const KODE::License &l )
{
  mFile.setLicense( l );
}

void Creator::setExportDeclaration( const QString &name )
{
  mExportDeclaration = name;
}

void Creator::setExternalClassPrefix( const QString &prefix )
{
  mExternalClassPrefix = prefix;

  setExternalClassNames();
}

void Creator::setExternalClassNames()
{
  mParserClass.setName( mExternalClassPrefix + "Parser" );
  mWriterClass.setName( mExternalClassPrefix + "Writer" );
}

KODE::File &Creator::file()
{
  return mFile;
}

void Creator::createProperty( KODE::Class &c,
  const ClassDescription &description, const QString &type,
  const QString &name )
{
  if ( type.startsWith( "Q" ) ) {
    c.addHeaderInclude( type );
  }

  KODE::MemberVariable v( Namer::getClassName( name ), type );
  c.addMemberVariable( v );

  KODE::Function mutator( Namer::getMutator( name ), "void" );
  if ( type == "int" ) {
    mutator.addArgument( type + " v" );
  } else {
    mutator.addArgument( "const " + type + " &v" );
  }
  mutator.addBodyLine( v.name() + " = v;" );
  if ( mCreateCrudFunctions ) {
    if ( name != "UpdatedAt" && name != "CreatedAt" ) {
      if ( description.hasProperty( "UpdatedAt" ) ) {
        mutator.addBodyLine( "setUpdatedAt( QDateTime::currentDateTime() );" );
      }
    }
  }

  if (name.toLower() == "value") {
    mutator.addBodyLine( "mValueHadBeenSet = true;" );
  }

  // do not pullote the code with the setValueHadBeenSet function
  // because that variable should be set only by the constructor
  // or the setValue method
  if (!(type == "bool" && name == "valueHadBeenSet")) {
    c.addFunction( mutator );
  }

  KODE::Function accessor( Namer::getAccessor( name ), type );
  accessor.setConst( true );
  if ( type.right(4) == "Enum" ) {
    accessor.setReturnType( c.name() + "::" + type );
  }

  accessor.addBodyLine( "return " + v.name() + ';' );
  c.addFunction( accessor );
}

void Creator::createCrudFunctions( KODE::Class &c, const QString &type )
{
  if ( !c.hasEnum( "Flags" ) ) {
    QList<XSD::SimpleType::EnumItem> enumValues;
    enumValues << XSD::SimpleType::EnumItem("None") << XSD::SimpleType::EnumItem("AutoCreate");
    KODE::Enum flags( "Flags", enumValues );
    c.addEnum( flags );
  }

  KODE::Function finder( "find" + type, type );

  finder.addArgument( "const QString &id" );
  finder.addArgument( KODE::Function::Argument( "Flags flags", "Flags_None" ) );

  QString listMember = "m" + type + "List";

  KODE::Code code;
  code += "foreach( " + type + " v, " + listMember + " ) {";
  code += "  if ( v.id() == id ) return v;";
  code += "}";
  code += type + " v;";
  code += "if ( flags == Flags_AutoCreate ) {";
  code += "  v.setId( id );";
  code += "}";
  code += "return v;";

  finder.setBody( code );

  c.addFunction( finder );

  KODE::Function inserter( "insert", "bool" );

  code.clear();

  inserter.addArgument( "const " + type + " &v" );

  code += "int i = 0;";
  code += "for( ; i < " + listMember + ".size(); ++i ) {";
  code += "  if ( " + listMember + "[i].id() == v.id() ) {";
  code += "    " + listMember + "[i] = v;";
  code += "    return true;";
  code += "  }";
  code += "}";
  code += "if ( i == " + listMember + ".size() ) {";
  code += "  add" + type + "( v );";
  code += "}";
  code += "return true;";

  inserter.setBody( code );

  c.addFunction( inserter );

  KODE::Function remover( "remove", "bool" );

  remover.addArgument( "const " + type + " &v" );

  code.clear();

  code += type + "::List::Iterator it;";
  code += "for( it = " + listMember + ".begin(); it != " + listMember +
    ".end(); ++it ) {";
  code += "  if ( (*it).id() == v.id() ) break;";
  code += "}";
  code += "if ( it != " + listMember + ".end() ) {";
  code += "  " + listMember + ".erase( it );";
  code += "}";
  code += "return true;";

  remover.setBody( code );

  c.addFunction( remover );
}


ClassDescription Creator::createClassDescription(
  const Schema::Element &element )
{
  ClassDescription description( Namer::getClassName( element ) );

  foreach( Schema::Relation r, element.attributeRelations() ) {
    Schema::Attribute a = mDocument.attribute( r );
    if ( a.enumerationValues().count() ) {
      if ( !description.hasEnum( a.name()) ) {
        description.addEnum(KODE::Enum(Namer::getClassName( a.name() ) + "Enum", a.enumerationValues()));
      }
      description.addProperty( Namer::getClassName( a.name() ) + "Enum",
                               Namer::getClassName( a.name() ) );
    } else {
      description.addProperty( typeName( a.type() ),
                               Namer::getClassName( a.name() ) );
    }
  }

  if ( element.type() > Schema::Node::None && element.type() < Schema::Node::Enumeration ) {
    description.addProperty( typeName( element.type() ), "value" );
  } else if ( element.type() == Schema::Node::Enumeration ) {
    description.addProperty( Namer::getClassName( element ) + "_Enum", "value" );
  }

  foreach( Schema::Relation r, element.elementRelations() ) {
    Schema::Element targetElement = mDocument.element( r );

    QString targetClassName = Namer::getClassName( targetElement );

    if ( targetElement.text() && !targetElement.hasAttributeRelations() &&
         !r.isList() ) {
      if ( mVerbose ) {
        qDebug() << "  FLATTEN";
      }
      if ( targetElement.type() == Schema::Element::Int ) {
        description.addProperty( "qint32", targetClassName );
      } else if ( targetElement.type() == Schema::Element::Integer ) {
        description.addProperty( "qlonglong", targetClassName );
      } else if ( targetElement.type() == Schema::Element::Short ) {
        description.addProperty( "qint16", targetClassName );
      } else if ( targetElement.type() == Schema::Element::Decimal ) {
        description.addProperty( "double", targetClassName );
      } else if ( targetElement.type() == Schema::Element::Date ) {
        description.addProperty( "QDate", targetClassName );
      } else if ( targetElement.type() == Schema::Element::Byte ) {
        description.addProperty( "quint8", targetClassName );
      } else {
        description.addProperty( "QString", targetClassName );
      }
    } else {
      if ( !mFile.hasClass( targetClassName ) ) {
        createClass( targetElement );
      }

      QString name = KODE::Style::lowerFirst( targetClassName );

      if ( r.isList() ) {
        ClassProperty p( targetClassName, Namer::getClassName( name ) );
        p.setIsList( true );
        
        ClassFlags targetClassFlags( targetElement );
        if ( targetClassFlags.hasId() ) {
          p.setTargetHasId( true );
        }

        p.setIsOptionalElement( r.isOptional() );
        description.addProperty( p );
      } else {
        description.addProperty( targetClassName, Namer::getClassName( name ), r.isOptional() );
      }
    }
  }
  
  return description;
}

void Creator::addCRUDConstructorCode(
        const ClassDescription & description,
        KODE::Code & code)
{
  bool hasCreatedAt = description.hasProperty( "CreatedAt" );
  bool hasUpdatedAt = description.hasProperty( "UpdatedAt" );
  if ( hasCreatedAt || hasUpdatedAt ) {
    code += "QDateTime now = QDateTime::currentDateTime();";
    if ( hasCreatedAt ) {
      code += "setCreatedAt( now );";
    }
    if ( hasUpdatedAt ) {
      code += "setUpdatedAt( now );";
    }
  }
}

void Creator::createConstructorOptionalMemberInitializator(
    const ClassDescription & description,
    KODE::Code & code )
{
  foreach( ClassProperty p, description.properties() ) {
    if ( p.isOptionalElement() && p.name().toLower() != "value") {
      code += 'm' + KODE::Style::upperFirst( p.name() ) + ".setElementIsOptional( true );";
    }
  }
}

void Creator::createCRUDIsValid(KODE::Class & c, ClassDescription & description)
{
  if ( description.hasProperty( "Id" ) ) {
    KODE::Function isValid( "isValid", "bool" );
    isValid.setConst( true );
    KODE::Code code;
    code += "return !mId.isEmpty();";
    isValid.setBody( code );
    c.addFunction( isValid );
  }
}

void Creator::createClass(const Schema::Element &element )
{
  QString className = Namer::getClassName( element  );
  if ( mVerbose ) {
    if ( element.type() == Schema::Node::Enumeration )
      qDebug() <<"Creator::createClass()" << element.identifier() << className << "ENUM";
    else
      qDebug() <<"Creator::createClass()" << element.identifier() << className;
    foreach( Schema::Relation r, element.elementRelations() ) {
      qDebug() << "  SUBELEMENTS" << r.target();
    }
  }

  if ( mProcessedClasses.contains( className ) ) {
    if ( mVerbose ) {
      qDebug() << "  ALREADY DONE";
    }
    return;
  }

  mProcessedClasses.append( className );

  ClassDescription description = createClassDescription( element );

  KODE::Class c( className );
  if ( !mExportDeclaration.isEmpty() ) {
    c.setExportDeclaration( mExportDeclaration );
  }

  if ((element.type() > Schema::Node::None && element.type() <= Schema::Node::Enumeration) ||
      element.type() == Schema::Node::ComplexType) {
      KODE::Function constructorDefault( className, "" );
      KODE::Code defaultCode;
      // initialize number based elements
      // FIXME/TODO add initializer rather
      if ( element.type() == Schema::Node::Decimal ||
           element.type() == Schema::Node::Int ||
           element.type() == Schema::Node::Byte ||
           element.type() == Schema::Node::Short ||
           element.type() == Schema::Node::Integer ) {
        defaultCode += "mValue = 0;";
      } else if ( element.type() == Schema::Node::Enumeration ) {
        QString typeNameStr = typeName( element ); // going to be suffixed with _Enum
        defaultCode += "mValue = " + typeNameStr.left( typeNameStr.length() - 5 ) + "__Invalid;";
      } else if ( element.type() == Schema::Node::Boolean ) {
        defaultCode += "mValue = false;";
      }
      defaultCode += "mValueHadBeenSet = false;";
      defaultCode += "mElementIsOptional = false;";
      createConstructorOptionalMemberInitializator( description, defaultCode );
      if (mCreateCrudFunctions)
        addCRUDConstructorCode( description, defaultCode );
      constructorDefault.setBody(defaultCode);
      c.addFunction(constructorDefault);

      // if this is a simple type create a default constructor for passing the value
      if (element.type() != Schema::Node::ComplexType) {
        KODE::Function constructor( className, "" );
        constructor.addArgument("const " + typeName( element ) +" &v");
        KODE::Code code; // FIXME add initializer rather
        code += "mValue = v;";
        code += "mValueHadBeenSet = true;";
        code += "mElementIsOptional = false;";
        createConstructorOptionalMemberInitializator( description, code );
        if (mCreateCrudFunctions)
          addCRUDConstructorCode( description, code );
        constructor.setBody(code);
        c.addFunction( constructor );
      }

      if (mCreateCrudFunctions)
        createCRUDIsValid(c, description);
  } else {
    if (mCreateCrudFunctions) {
      bool hasCreatedAt = description.hasProperty( "CreatedAt" );
      bool hasUpdatedAt = description.hasProperty( "UpdatedAt" );
      if ( hasCreatedAt || hasUpdatedAt ) {
        KODE::Function constructor( className, "" );
        KODE::Code code;
        code += "QDateTime now = QDateTime::currentDateTime();";
        if ( hasCreatedAt ) {
          code += "setCreatedAt( now );";
        }
        if ( hasUpdatedAt ) {
          code += "setUpdatedAt( now );";
        }
        constructor.setBody( code );
        c.addFunction( constructor );

        createCRUDIsValid(c, description);
      }
    }
  }

  if ( mCreateCrudFunctions ) {
    if ( description.hasProperty( "Id" ) ) {
      KODE::Function isValid( "isValid", "bool" );
      isValid.setConst( true );
      KODE::Code code;
      code += "return !mId.isEmpty();";
      isValid.setBody( code );
    }
  }

  foreach( ClassProperty p, description.properties() ) {
    if ( p.isList() ) {
      registerListTypedef( p.type() );

      c.addHeaderInclude( "QList" );
      QString listName = p.name() + "List";

      KODE::Function adder( "add" + p.type(), "void" );
      adder.addArgument( "const " + p.type() + " &v" );

      KODE::Code code;
      code += 'm' + KODE::Style::upperFirst( listName ) + ".append( v );";

      adder.setBody( code );

      c.addFunction( adder );

      createProperty( c, description, p.type() + "::List", listName );

      if ( mCreateCrudFunctions && p.targetHasId() ) {
        createCrudFunctions( c, p.type() );
      }
    } else {
      createProperty( c, description, p.type(), p.name() );
    }
  }

  // silly member name had been choosen to make the possible conflicts
  // with existing members low
  createProperty( c, description, "bool", "valueHadBeenSet" );
  createProperty( c, description, "bool", "elementIsOptional");
  
  foreach( KODE::Enum e, description.enums() ) {
    c.addEnum(e);
  }

  if ( element.type() == Schema::Node::Enumeration ) {
    QList<XSD::SimpleType::EnumItem> enumItems = element.enumerationValues();
    KODE::Enum selfEnum( typeName( element ), enumItems, false);
    c.addEnum(selfEnum);
  }

  if ( m_createParseFunctions)
    createElementParser( c, element );
  
  if ( m_createWriteFunctions ) {
    WriterCreator writerCreator( mFile, mDocument, mDtd );
    writerCreator.createElementWriter( c, element, mDocument.targetNamespace() );
  }
  mFile.insertClass( c );
}

void Creator::createElementParser( KODE::Class &c, const Schema::Element &e )
{
  ParserCreator *parserCreator = 0;

  switch ( mXmlParserType ) {
    case XmlParserDom:
    case XmlParserDomExternal:
      parserCreator = new ParserCreatorDom( this );
      break;
  }

  if ( parserCreator == 0 ) {
    return;
  }

  parserCreator->createElementParser( c, e );

  delete parserCreator;
}

void Creator::registerListTypedef( const QString &type )
{
  if ( !mListTypedefs.contains( type ) ) mListTypedefs.append( type );
}

void Creator::createListTypedefs()
{
  QStringList::ConstIterator it;
  for( it = mListTypedefs.constBegin(); it != mListTypedefs.constEnd(); ++it ) {
    KODE::Class c = mFile.findClass( *it );
    if ( !c.isValid() ) continue;
    c.addTypedef( KODE::Typedef( "QList<" + *it + '>', "List" ) );
    mFile.insertClass( c );
  }
}

void Creator::setDtd( const QString &dtd )
{
  mDtd = dtd;
}

void Creator::createFileWriter( const Schema::Element &element )
{
  WriterCreator writerCreator( mFile, mDocument, mDtd );
  writerCreator.createFileWriter( Namer::getClassName( element ),
    errorStream() );
}

void Creator::createFileParser( const Schema::Element &element )
{
  ParserCreator *parserCreator = 0;

  switch ( mXmlParserType ) {
    case XmlParserDom:
    case XmlParserDomExternal:
      parserCreator = new ParserCreatorDom( this );
      break;
  }

  parserCreator->createFileParser( element );
  parserCreator->createStringParser( element );

  delete parserCreator;
}

void Creator::printFiles( KODE::Printer &printer )
{
  if ( externalParser() ) {
    KODE::File parserFile( file() );
    parserFile.setFilename( mBaseName + "_parser" );

    parserFile.clearCode();

    mParserClass.addHeaderInclude( file().filenameHeader() );
    parserFile.insertClass( mParserClass );

    if ( mVerbose ) {
      qDebug() <<"Print external parser header" << parserFile.filenameHeader();
    }
    printer.printHeader( parserFile );
    if ( mVerbose ) {
      qDebug() <<"Print external parser implementation"
        << parserFile.filenameImplementation();
    }
    printer.printImplementation( parserFile );
  }

  if ( mVerbose ) {
    qDebug() <<"Print header" << file().filenameHeader();
  }
  printer.printHeader( file() );

  if ( mVerbose ) {
    qDebug() <<"Print implementation" << file().filenameImplementation();
  }
  printer.printImplementation( file() );

}

bool Creator::externalParser() const
{
  return mXmlParserType == XmlParserDomExternal;
}

bool Creator::externalWriter() const
{
  return false;
}

const Schema::Document &Creator::document() const
{
  return mDocument;
}

void Creator::setParserClass( const KODE::Class &c )
{
  mParserClass = c;
}

KODE::Class &Creator::parserClass()
{
  return mParserClass;
}

QString Creator::errorStream() const
{
  if ( useKde() ) {
    return "qDebug()";
  } else {
    return "qCritical()";
  }
}

QString Creator::debugStream() const
{
  if ( useKde() ) {
    return "qDebug()";
  } else {
    return "qDebug()";
  }
}

void Creator::create()
{
  Schema::Element startElement = mDocument.startElement();
  setExternalClassPrefix( KODE::Style::upperFirst( startElement.name() ) );
  if ( m_createParseFunctions )
    createFileParser( startElement );
//  setDtd( schemaFilename.replace( "rng", "dtd" ) );
  if ( m_createWriteFunctions )
   createFileWriter( startElement );

  createListTypedefs();
}

void Creator::setFilename( const QString& baseName )
{
  mFile.setFilename( baseName );
  mBaseName = baseName;
}

QString Creator::typeName( Schema::Node::Type type )
{
  if ( type == Schema::Element::DateTime ) {
    return "QDateTime";
  } else if ( type == Schema::Element::Date ) {
    return "QDate";
  } else if ( type == Schema::Element::Int ) {
    return "qint32";
  } else if ( type == Schema::Element::Integer ) {
    return "qlonglong";
  } else if ( type == Schema::Element::Decimal ) {
    return "double";
  } else if ( type == Schema::Element::Byte ) {
    return "quint8";
  } else if ( type == Schema::Element::Short ) {
    return "qint16";
  } else if ( type == Schema::Element::String ||
              type == Schema::Element::NormalizedString ||
              type == Schema::Element::Token ) {
    return "QString";
  } else if ( type == Schema::Element::Boolean) {
    return "bool";
  } else {
    return "Invalid";
  }
}

QString Creator::typeName(const Schema::Element &element)
{
  if ( element.type() == Schema::Node::Enumeration ) {
    return Namer::getClassName( element ) + "_Enum";
  } else {
    return typeName( element.type() );
  }
}
bool Creator::createWriteFunctions() const
{
  return m_createWriteFunctions;
}

void Creator::setCreateWriteFunctions(bool createWriteFunctions)
{
  m_createWriteFunctions = createWriteFunctions;
}

bool Creator::createParseFunctions() const
{
  return m_createParseFunctions;
}

void Creator::setCreateParseFunctions(bool createParseFunctions)
{
  m_createParseFunctions = createParseFunctions;
}



ParserCreator::ParserCreator( Creator *c )
  : mCreator( c )
{
}

ParserCreator::~ParserCreator()
{
}

Creator *ParserCreator::creator() const
{
  return mCreator;
}

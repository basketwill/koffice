/***************************************************************************
 * pythonvariant.h
 * This file is part of the KDE project
 * copyright (C)2004-2005 by Sebastian Sauer (mail@dipe.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef KROSS_PYTHONVARIANT_H
#define KROSS_PYTHONVARIANT_H

#include "pythonconfig.h"
#include <kross/core/metatype.h>

#include <QString>
#include <QStringList>
#include <QVariant>
#include <QMetaType>

#include <QSize>
#include <QPoint>
#include <QRect>

#include <QDate>
#include <QTime>
#include <QDateTime>

#include <typeinfo>

namespace Kross {

    /**
     * The PythonType helper classes used to cast between QVariant
     * and Py::Object values.
     *
     * Following QVariant::Type's are implemented;
     *   \li QVariant::Invalid
     *   \li QVariant::Int
     *   \li QVariant::UInt
     *   \li QVariant::Double
     *   \li QVariant::Bool
     *   \li QVariant::LongLong
     *   \li QVariant::ULongLong
     *   \li QVariant::ByteArray
     *   \li QVariant::Size
     *   \li QVariant::SizeF
     *   \li QVariant::Point
     *   \li QVariant::PointF
     *   \li QVariant::Rect
     *   \li QVariant::RectF
     *   \li QVariant::String
     *   \li QVariant::StringList
     *   \li QVariant::List
     *   \li QVariant::Map
     *
     * Following QVariant::Type's are unimplemented yet (do we need them anyways?);
     *   \li QVariant::BitArray
     *   \li QVariant::Date
     *   \li QVariant::Time
     *   \li QVariant::DateTime
     *   \li QVariant::Bitmap
     *   \li QVariant::Brush
     *   \li QVariant::Char
     *   \li QVariant::Color
     *   \li QVariant::Cursor
     *   \li QVariant::Font
     *   \li QVariant::Icon
     *   \li QVariant::Image
     *   \li QVariant::KeySequence
     *   \li QVariant::Line
     *   \li QVariant::LineF
     *   \li QVariant::Locale
     *   \li QVariant::Palette
     *   \li QVariant::Pen
     *   \li QVariant::Pixmap
     *   \li QVariant::PointArray
     *   \li QVariant::Polygon
     *   \li QVariant::RegExp
     *   \li QVariant::Region
     *   \li QVariant::SizePolicy
     *   \li QVariant::TextFormat
     *   \li QVariant::TextLength
     *   \li QVariant::Url
     */
    template<typename VARIANTTYPE, typename PYTYPE = Py::Object>
    struct PythonType
    {
        // template-specialisations need to implement following both static
        // functions to translate between QVariant and Py::Object objects.

        //inline static PYTYPE toPyObject(const VARIANTTYPE&) { return Py::None(); }
        //inline static QVARIANTTYPE toVariant(const VARIANTTYPE&) { return QVariant(); }
    };

    /// \internal
    template<>
    struct PythonType<QVariant>
    {
        static Py::Object toPyObject(const QVariant& v);
        static QVariant toVariant(const Py::Object& obj);
    };

    /// \internal
    template<>
    struct PythonType<int>
    {
        inline static Py::Object toPyObject(int i) {
            return Py::Int(i);
        }
        inline static int toVariant(const Py::Object& obj) {
            return int(Py::Int(obj));
        }
    };

    /// \internal
    template<>
    struct PythonType<uint>
    {
        inline static Py::Object toPyObject(uint i) {
            return Py::Long( (unsigned long)i );
        }
        inline static uint toVariant(const Py::Object& obj) {
            return uint( (unsigned long)Py::Long(obj) );
        }
    };

    /// \internal
    template<>
    struct PythonType<double>
    {
        inline static Py::Object toPyObject(double d) {
            return Py::Float(d);
        }
        inline static double toVariant(const Py::Object& obj) {
            return double(Py::Float(obj));
        }
    };

    /// \internal
    template<>
    struct PythonType<bool>
    {
        inline static Py::Object toPyObject(bool b) {
            return Py::Int(b);
        }
        inline static double toVariant(const Py::Object& obj) {
            return bool(Py::Int(obj));
        }
    };

    /// \internal
    template<>
    struct PythonType<qlonglong>
    {
        inline static Py::Object toPyObject(qlonglong l) {
            return Py::Long( (long)l );
        }
        inline static qlonglong toVariant(const Py::Object& obj) {
            return (long) Py::Long(obj);
        }
    };

    /// \internal
    template<>
    struct PythonType<qulonglong>
    {
        inline static Py::Object toPyObject(qulonglong l) {
            return Py::Long( (unsigned long)l );
        }
        inline static qulonglong toVariant(const Py::Object& obj) {
            return (unsigned long) Py::Long(obj);
        }
    };

    /// \internal
    template<>
    struct PythonType<QByteArray>
    {
        inline static Py::Object toPyObject(const QByteArray& ba) {
            return Py::String(ba.constData(), ba.size());
        }
        inline static QByteArray toVariant(const Py::Object& obj) {
            char* s = PyString_AS_STRING(obj.ptr());
            int size = PyString_Size(obj.ptr());
            return QByteArray(s, size);
        }
    };

    /// \internal
    template<>
    struct PythonType<QString>
    {
        inline static Py::Object toPyObject(const QString& s) {
            return s.isNull() ? Py::String() : Py::String(s.toLatin1().data());
        }
        inline static QString toVariant(const Py::Object& obj) {
            /*
            #ifdef Py_USING_UNICODE
                //if(obj.isUnicode())
                PyTypeObject *type = (PyTypeObject*) object.type().ptr();
                if(type == &PyUnicode_Type) {
                    Py::unicodestring u = Py::String(object).as_unicodestring();
                    std::string s;
                    std::copy(u.begin(), u.end(), std::back_inserter(s));
                    return s.c_str();
                }
            #endif
            */
            return obj.isString() ? Py::String(obj).as_string().c_str() : QString();
        }
    };

    /// \internal
    template<>
    struct PythonType<QStringList>
    {
        inline static Py::Object toPyObject(const QStringList& sl) {
            Py::List l;
            foreach(QString s, sl)
                l.append( PythonType<QString>::toPyObject(s) );
            return l;
        }
        inline static QStringList toVariant(const Py::Object& obj) {
            Py::List list(obj);
            QStringList l;
            const uint length = list.length();
            for(uint i = 0; i < length; i++)
                l.append( Py::String(list[i]).as_string().c_str() );
            return l;
        }
    };

    /// \internal
    template<>
    struct PythonType<QVariantList,Py::List>
    {
        inline static Py::List toPyObject(const QVariantList& list) {
            Py::List l;
            foreach(QVariant v, list)
                l.append( PythonType<QVariant>::toPyObject(v) );
            return l;
        }
        inline static QVariantList toVariant(const Py::List& list) {
            QVariantList l;
            const uint length = list.length();
            for(uint i = 0; i < length; i++)
                l.append( PythonType<QVariant>::toVariant(list[i]) );
            return l;
        }
    };

    /// \internal
    template<>
    struct PythonType<QVariantList>
    {
        inline static Py::Object toPyObject(const QVariantList& list) {
            return PythonType<QVariantList,Py::List>::toPyObject(list);
        }
        inline static QVariantList toVariant(const Py::Object& obj) {
            return PythonType<QVariantList,Py::List>::toVariant(Py::List(obj));
        }
    };

    /// \internal
    template<>
    struct PythonType<QVariantList,Py::Tuple>
    {
        inline static Py::Tuple toPyObject(const QVariantList& list) {
            uint count = list.count();
            Py::Tuple tuple(count);
            for(uint i = 0; i < count; ++i)
                tuple.setItem(i, PythonType<QVariant>::toPyObject(list[i]));
            return tuple;
        }
        inline static QVariantList toVariant(const Py::Tuple& tuple) {
            QVariantList l;
            const uint size = tuple.size();
            for(uint i = 0; i < size; i++)
                l.append( PythonType<QVariant>::toVariant(tuple[i]) );
            return l;
        }
    };

    /// \internal
    template<>
    struct PythonType<QVariantMap,Py::Dict>
    {
        inline static Py::Dict toPyObject(const QVariantMap& map) {
            Py::Dict d;
            for(QMap<QString, QVariant>::ConstIterator it = map.constBegin(); it != map.constEnd(); ++it)
                d.setItem( it.key().toLatin1().data(), PythonType<QVariant>::toPyObject(it.value()) );
            return d;
        }
        inline static QVariantMap toVariant(const Py::Dict& dict) {
            QVariantMap map;
            Py::List l = dict.keys();
            const uint length = l.length();
            for(Py::List::size_type i = 0; i < length; ++i) {
                const char* n = l[i].str().as_string().c_str();
                map.insert( n, PythonType<QVariant>::toVariant(dict[n]) );
            }
            return map;
        }
    };

    /// \internal
    template<>
    struct PythonType<QVariantMap>
    {
        inline static Py::Object toPyObject(const QVariantMap& map) {
            return PythonType<QVariantMap,Py::Dict>::toPyObject(map);
        }
        inline static QVariantMap toVariant(const Py::Object& obj) {
            return PythonType<QVariantMap,Py::Dict>::toVariant( Py::Dict(obj.ptr()) );
        }
    };

    /// \internal
    template<>
    struct PythonType<QSize>
    {
        inline static Py::Object toPyObject(const QSize& size) {
            Py::List list;
            list.append( PythonType<int>::toPyObject(size.width()) );
            list.append( PythonType<int>::toPyObject(size.height()) );
            return list;
        }
        inline static QSize toVariant(const Py::Object& obj) {
            Py::List list(obj);
            return QSize(PythonType<int>::toVariant(list[0]), PythonType<int>::toVariant(list[1]));
        }
    };

    /// \internal
    template<>
    struct PythonType<QSizeF>
    {
        inline static Py::Object toPyObject(const QSizeF& size) {
            Py::List list;
            list.append( PythonType<double>::toPyObject(size.width()) );
            list.append( PythonType<double>::toPyObject(size.height()) );
            return list;
        }
        inline static QSizeF toVariant(const Py::Object& obj) {
            Py::List list(obj);
            return QSizeF(PythonType<double>::toVariant(list[0]), PythonType<double>::toVariant(list[1]));
        }
    };

    /// \internal
    template<>
    struct PythonType<QPoint>
    {
        inline static Py::Object toPyObject(const QPoint& point) {
            Py::List list;
            list.append( PythonType<int>::toPyObject(point.x()) );
            list.append( PythonType<int>::toPyObject(point.y()) );
            return list;
        }
        inline static QPoint toVariant(const Py::Object& obj) {
            Py::List list(obj);
            return QPoint(PythonType<int>::toVariant(list[0]), PythonType<int>::toVariant(list[1]));
        }
    };

    /// \internal
    template<>
    struct PythonType<QPointF>
    {
        inline static Py::Object toPyObject(const QPointF& point) {
            Py::List list;
            list.append( PythonType<double>::toPyObject(point.x()) );
            list.append( PythonType<double>::toPyObject(point.y()) );
            return list;
        }
        inline static QPointF toVariant(const Py::Object& obj) {
            Py::List list(obj);
            return QPointF(PythonType<double>::toVariant(list[0]), PythonType<double>::toVariant(list[1]));
        }
    };

    /// \internal
    template<>
    struct PythonType<QRect>
    {
        inline static Py::Object toPyObject(const QRect& rect) {
            Py::List list;
            list.append( PythonType<int>::toPyObject(rect.x()) );
            list.append( PythonType<int>::toPyObject(rect.y()) );
            list.append( PythonType<int>::toPyObject(rect.width()) );
            list.append( PythonType<int>::toPyObject(rect.height()) );
            return list;
        }
        inline static QRect toVariant(const Py::Object& obj) {
            Py::List list(obj);
            return QRect(PythonType<int>::toVariant(list[0]), PythonType<int>::toVariant(list[1]),
                         PythonType<int>::toVariant(list[2]), PythonType<int>::toVariant(list[3]));
        }
    };

    /// \internal
    template<>
    struct PythonType<QRectF>
    {
        inline static Py::Object toPyObject(const QRectF& rect) {
            Py::List list;
            list.append( PythonType<double>::toPyObject(rect.x()) );
            list.append( PythonType<double>::toPyObject(rect.y()) );
            list.append( PythonType<double>::toPyObject(rect.width()) );
            list.append( PythonType<double>::toPyObject(rect.height()) );
            return list;
        }
        inline static QRectF toVariant(const Py::Object& obj) {
            Py::List list(obj);
            return QRectF(PythonType<double>::toVariant(list[0]), PythonType<double>::toVariant(list[1]),
                          PythonType<double>::toVariant(list[2]), PythonType<double>::toVariant(list[3]));
        }
    };

    // this is a testcase and not activly used yet cause there seems to be still some
    // open issues. Please see also the unittest.py which contains tests for all cases.
    /// \internal
    template<>
    struct PythonType<QColor>
    {
        static Py::Object toPyObject(const QColor& color);
        static QColor toVariant(const Py::Object& obj);
    };

#if 0
    /// \internal
    template<>
    struct PythonType<QFont>
    {
        inline static Py::Object toPyObject(const QFont& font) {
            //TODO
            return Py::asObject(new PythonExtension(new Font(font)));
        }
        inline static QFont toVariant(const Py::Object& obj) {
            //TODO
        }
    };

    /// \internal
    template<>
    struct PythonType<QDateTime>
    {
        inline static Py::Object toPyObject(const QDateTime& font) {
            //TODO
        }
        inline static QDateTime toVariant(const Py::Object& obj) {
            //TODO
        }
    };

    /// \internal
    template<>
    struct PythonType<QTime>
    {
        inline static Py::Object toPyObject(const QTime& font) {
            //TODO
        }
        inline static QTime toVariant(const Py::Object& obj) {
            //TODO
        }
    };

    /// \internal
    template<>
    struct PythonType<QDate>
    {
        inline static Py::Object toPyObject(const QDate& font) {
            //TODO
        }
        inline static QDate toVariant(const Py::Object& obj) {
            //TODO
        }
    };

    /*
    /// \internal
    template<>
    struct PythonType<QDate>
    {
        inline static Py::Object toPyObject(const QDate& d) {
            Py::Module mod( PyImport_ImportModule((char*)"datetime"), true );
            PyObject* result = PyObject_CallMethod(mod.ptr(),(char*)"date","iii",d.year(),d.month(),d.day());
            return Py::Object(result, true);
        }
        inline static QDate toVariant(const Py::Object& obj) {
            return QDate( Py::Int(obj.getAttr("year")), Py::Int(obj.getAttr("month")), Py::Int(obj.getAttr("day")) );
        }
    };

    /// \internal
    template<>
    struct PythonType<QTime>
    {
        inline static Py::Object toPyObject(const QTime& t) {
            Py::Module mod( PyImport_ImportModule((char*)"datetime"), true );
            PyObject* result = PyObject_CallMethod(mod.ptr(),(char*)"time",(char*)"iii",t.hour(),t.minute(),t.second());
            return Py::Object(result, true);
        }
        inline static QTime toVariant(const Py::Object& obj) {
            return QTime( Py::Int(obj.getAttr("hour")), Py::Int(obj.getAttr("minute")), Py::Int(obj.getAttr("second")) );
        }
    };

    /// \internal
    template<>
    struct PythonType<QDateTime>
    {
        inline static Py::Object toPyObject(const QDateTime& dt) {
            QDate d = dt.date();
            QTime t = dt.time();
            Py::Module mod( PyImport_ImportModule((char*)"datetime"), true );
            PyObject* result = PyObject_CallMethod(mod.ptr(),(char*)"datetime",(char*)"iiiiii",d.year(),d.month(),d.day(),t.hour(),t.minute(),t.second());
            return Py::Object(result, true);
        }
        inline static QDateTime toVariant(const Py::Object& obj) {
            return QDateTime( PythonType<QDate>::toVariant(obj), PythonType<QTime>::toVariant(obj) );
        }
    };
    */
#endif

    /*
    /// \internal
    template<>
    struct PythonType<QObject>
    {
        inline static Py::Object toPyObject(QObject* obj) {
            return Py::asObject( new PythonExtension(obj) );
        }
        inline static QVariant toVariant(int typeId, const Py::Object& obj) {
            if(Py::PythonExtension<PythonExtension>::check( obj )) {
                Py::ExtensionObject<PythonExtension> extobj(object);
                PythonExtension* extension = extobj.extensionObject();
                return QVariant(typeId, extension->object());
            }
            return QVariant();
        }
        inline static QVariant toVariant(const Py::Object& obj) {
            if(Py::PythonExtension<PythonExtension>::check( obj )) {
                Py::ExtensionObject<PythonExtension> extobj(object);
                PythonExtension* extension = extobj.extensionObject();
                QObject* o = extension->object();
                int typeId = QMetaType::type( o->metaObject()->className() );
                return QVariant(typeId, o);
            }
            return QVariant();
        }
    };
    */

    /*
    /// \internal
    template<>
    struct PythonType< void* >
    {
        inline static Py::Object toPyObject(void* p) {
            PyObject* pyobj = PyLong_FromVoidPtr(p);
            return Py::asObject(pyobj);
        }
        inline static QVariant toVariant(int varianttypeid, const Py::Long& obj) {
            void* p = PyLong_AsVoidPtr(obj.ptr());
            return QVariant(varianttypeid, p);
        }
    };
    */

    /**
     * The PythonMetaTypeFactory helper class is used as factory within
     * \a PythonExtension to translate an argument into a \a MetaType
     * needed for QGenericArgument's data pointer.
     */
    class PythonMetaTypeFactory
    {
        public:
            static MetaType* create(const char* typeName, const Py::Object& object);
    };

    /// \internal
    template<typename VARIANTTYPE>
    class PythonMetaTypeVariant : public MetaTypeVariant<VARIANTTYPE>
    {
        public:
            PythonMetaTypeVariant(const Py::Object& obj)
                : MetaTypeVariant<VARIANTTYPE>( PythonType<VARIANTTYPE>::toVariant(obj) ) {}
            virtual ~PythonMetaTypeVariant() {}
    };

}

#endif

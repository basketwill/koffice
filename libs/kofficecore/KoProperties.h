/*
   Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
 
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
#ifndef _KO_PROPERTIES_H
#define _KO_PROPERTIES_H

#include <QString>
#include <QMap>
#include <QVariant>
#include "koffice_export.h"


/**
 * A KoProperties is the serializable representation of
 * the filter parameters. Filters can subclass this class to implement
 * direct accessors to properties, but properties not in the map will
 * not be serialized.
 */
class KOFFICECORE_EXPORT KoProperties {

public:

    /**
     * Create a new filter config.
     */
    KoProperties() {}

    /**
     * Deep copy the filter configFile
     */
    KoProperties(const KoProperties & rhs);

    virtual ~KoProperties() {}

public:

    /**
     * Fill the filter configuration object from the XML encoded representation in s.
     */
    virtual void load(const QString &);


    /**
     * Create a serialized version of this filter config
     */
    virtual QString store();


    /**
     * Set the property with name to value.
     */
    virtual void setProperty(const QString & name, const QVariant & value);

    /**
     * Set value to the value associated with property name
     * @return false if the specified property did not exist.
     */
    virtual bool getProperty(const QString & name, QVariant & value) const;

    /**
     * Return a property by name, wrapped in a QVariant.
     * A typical usage:
     *  @code
     *      KoProperties *props = new KoProperties();
     *      props->setProperty("name", "Marcy");
     *      props->setProperty("age", 25);
     *      QString name = props->getProperty("name").toString();
     *      int age = props->getProperty("age").toInt();
     *  @endcode
     * @return a property by name, wrapped in a QVariant.
     * @param name the name (or key) with which the variant was registred.
     * @see getInt() getString()
     */
    virtual QVariant getProperty(const QString & name) const;

    /**
     * Return an integer property by name.
     * A typical usage:
     *  @code
     *      KoProperties *props = new KoProperties();
     *      props->setProperty("age", 25);
     *      int age = props->getInt("age");
     *  @endcode
     * @return an integer property by name
     * @param name the name (or key) with which the variant was registred.
     * @param def the default value, should there not be no propery by the name this will be returned.
     * @see getProperty() getString()
     */
    int getInt(const QString & name, int def = 0) const;
    /**
     * Return a double property by name.
     * @param name the name (or key) with which the variant was registred.
     * @param def the default value, should there not be no propery by the name this will be returned.
     */
    double getDouble(const QString & name, double def = 0.0) const;
    /**
     * Return a boolean property by name.
     * @param name the name (or key) with which the variant was registred.
     * @param def the default value, should there not be no propery by the name this will be returned.
     */
    bool getBool(const QString & name, bool def = false) const;
    /**
     * Return an QString property by name.
     * A typical usage:
     *  @code
     *      KoProperties *props = new KoProperties();
     *      props->setProperty("name", "Marcy");
     *      QString name = props->getString("name");
     *  @endcode
     * @return an QString property by name
     * @param name the name (or key) with which the variant was registred.
     * @see getProperty() getInt()
     * @param def the default value, should there not be no propery by the name this will be returned.
     */
    QString getString(const QString & name, QString def = QString::null) const;

private:
    void dump();

protected:

    QMap<QString, QVariant> m_properties;

};

#endif // _KO_PROPERTIES_H

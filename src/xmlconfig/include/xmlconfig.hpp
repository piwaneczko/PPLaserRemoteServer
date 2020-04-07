/**
 * \brief       Header of the XML configuration class.
 * \file        XmlConfig.hpp
 * \author      Pawe³ Iwaneczko
 * \copyright   Copyright 2016 HTeam. All rights reserved.
 */
#pragma once
#include <cassert>
#include <cstdint>
#include <map>
#include <string>
#include <type_traits>
#include "pugixml.hpp"

using namespace std;
using namespace pugi;

class XmlConfigGroup;
class XmlConfigElement {
    friend class XmlConfig;

private:
    XmlConfigGroup *parent;
    string name;

public:
    /**
     * \fn  XmlConfigElement::XmlConfigElement(const string &name, XmlConfigGroup *parent = nullptr);
     *
     * \brief   Constructor of xml configuration element.
     *
     * \author  Pawel Iwaneczko
     * \date    06.04.2016
     *
     * \param   name            The name of xml configuration element.
     * \param [in,out]  parent  (Optional) If non-null, the elements group parent.
     */

    XmlConfigElement(const string &name, XmlConfigGroup *parent = nullptr);

    /**
     * \fn  string XmlConfigElement::GetName() const;
     *
     * \brief   Gets the element name.
     *
     * \author  Pawel Iwaneczko
     * \date    07.04.2016
     *
     * \return  The element name.
     */

    string GetName() const;
};
class XmlConfigGroup : public XmlConfigElement {
    friend class XmlConfigElement;
    friend class XmlConfig;

private:
    map<string, XmlConfigElement *> elements;

public:
    /**
     * \fn  XmlConfigGroup::XmlConfigGroup(const string &name);
     *
     * \brief   Constructor of xml configuration group.
     *
     * \author  Pawel Iwaneczko
     * \date    06.04.2016
     *
     * \param   name        The name of xml configuration group.
     */
    XmlConfigGroup(const string &name);
};

class XmlConfig {
private:
    xml_document doc;
    string filePath;

    /**
     * \fn  XmlConfig::XmlConfig();
     *
     * \brief   Default constructor.
     *
     * \author  Pawel Iwaneczko
     * \date    06.04.2016
     */

    XmlConfig();

    /**
     * \fn  XmlConfig::~XmlConfig();
     *
     * \brief   Destructor.
     *
     * \author  Pawel Iwaneczko
     * \date    07.04.2016
     */

    ~XmlConfig();

public:
    /**
     * \fn  static XmlConfig XmlConfig::&GetInstance();
     *
     * \brief   Gets the singleton instance.
     *
     * \author  Pawel Iwaneczko
     * \date    07.04.2016
     *
     * \return  The singleton instance.
     */

    static XmlConfig &GetInstance();

    /**
     * \fn  static bool XmlConfig::FindElement(const XmlConfigElement &element, xml_attribute &attr);
     *
     * \brief   Searches for the element in xml configuration file.
     *
     * \author  Pawel Iwaneczko
     * \date    06.04.2016
     *
     * \param   element         The element to serach.
     * \param [in,out]  attr    The finded xml attribute result.
     *
     * \return  true if it succeeds, false if it fails.
     */
    static bool FindElement(const XmlConfigElement &element, xml_attribute &attr);

    /**
     * \fn  static void XmlConfig::UpdateXmlFile();
     *
     * \brief   Updates the XML file.
     *
     * \author  Pawel Iwaneczko
     * \date    07.04.2016
     */

    static void UpdateXmlFile();

    /**
     * \fn  static void XmlConfig::SetXMLFilePath(string filePath);
     *
     * \brief   Sets XML file path.
     *
     * \author  Pawel Iwaneczko
     * \date    27.04.2016
     *
     * \param   filePath    Full pathname of the file.
     */

    static void SetXMLFilePath(string filePath);

    /**
     * \fn  static string XmlConfig::GetXMLFilePath();
     *
     * \brief   Gets XML file path.
     *
     * \author  Pawel Iwaneczko
     * \date    27.04.2016
     *
     * \return  The XML file path.
     */

    static string GetXMLFilePath();
};

enum XmlConfigFlag { XmlConfigReadFlag = 0x01, XmlConfigWriteFlag = 0x02, XmlConfigReadWriteFlag = 0x03 };

template <typename T, XmlConfigFlag f = XmlConfigReadFlag>
class XmlConfigValue : public XmlConfigElement {
private:
    T value;
    void operator=(const XmlConfigValue &copy) {}

private:
    template <typename = typename enable_if<is_same<bool, T>::value>::type>
    void InitValue(xml_attribute &attr, bool get, bool &value) {
        if (get)
            value = attr.as_bool(value);
        else
            attr = value;
    }
    template <typename = typename enable_if<is_same<double, T>::value>::type>
    void InitValue(xml_attribute &attr, bool get, double &value) {
        if (get)
            value = attr.as_double(value);
        else
            attr = value;
    }
    template <typename = typename enable_if<is_same<float, T>::value>::type>
    void InitValue(xml_attribute &attr, bool get, float &value) {
        if (get)
            value = attr.as_float(value);
        else
            attr = value;
    }
    template <typename = typename enable_if<is_same<int32_t, T>::value>::type>
    void InitValue(xml_attribute &attr, bool get, int32_t &value) {
        if (get)
            value = attr.as_int(value);
        else
            attr = value;
    }
    template <typename = typename enable_if<is_same<int16_t, T>::value>::type>
    void InitValue(xml_attribute &attr, bool get, int16_t &value) {
        if (get)
            value = attr.as_int(value);
        else
            attr = value;
    }
    template <typename = typename enable_if<is_same<int8_t, T>::value>::type>
    void InitValue(xml_attribute &attr, bool get, int8_t &value) {
        if (get)
            value = attr.as_int(value);
        else
            attr = value;
    }
    template <typename = typename enable_if<is_same<string, T>::value>::type>
    void InitValue(xml_attribute &attr, bool get, string &value) {
        if (get)
            value = attr.as_string(value.c_str());
        else
            attr = value.c_str();
    }
    template <typename = typename enable_if<is_same<uint32_t, T>::value>::type>
    void InitValue(xml_attribute &attr, bool get, uint32_t &value) {
        if (get)
            value = attr.as_uint(value);
        else
            attr = value;
    }
    template <typename = typename enable_if<is_same<uint16_t, T>::value>::type>
    void InitValue(xml_attribute &attr, bool get, uint16_t &value) {
        if (get)
            value = attr.as_uint(value);
        else
            attr = value;
    }
    template <typename = typename enable_if<is_same<uint8_t, T>::value>::type>
    void InitValue(xml_attribute &attr, bool get, uint8_t &value) {
        if (get)
            value = attr.as_uint(value);
        else
            attr = value;
    }

public:
    /**
     * \fn  XmlConfigValue::XmlConfigValue(const string &name, const T &value = T(), XmlConfigGroup *parent = nullptr)
     *
     * \brief   Constructor of xml configuration field.
     *
     * \author  Pawel Iwaneczko
     * \date    06.04.2016
     *
     * \param   name            The name of xml configuration field.
     * \param   value           The value of xml configuration field.
     * \param [in,out]  parent  (Optional) If non-null, the elements group parent.
     */

    XmlConfigValue(const string &name, const T &initValue = T(), XmlConfigGroup *parent = nullptr) : XmlConfigElement(name, parent) {
        assert((f == XmlConfigReadFlag) || (f == XmlConfigWriteFlag) || (f == XmlConfigReadWriteFlag));
        xml_attribute attr;
        if (XmlConfig::FindElement(*this, attr))  // element istnieje - pobranie wartoœci z pliku xml
            InitValue(attr, true, this->value);
        else {  // element nie istnieje
            this->value = initValue;
            InitValue(attr, false, this->value);
            XmlConfig::UpdateXmlFile();
        }
    }

    /**
     * \fn  template < typename = typename XmlConfigValue::enable_if< (f & XmlConfigReadFlag) == XmlConfigReadFlag >::type > operator T() const
     *
     * \brief   Get the xml configuration field value.
     *
     * \author  Pawel Iwaneczko
     * \date    06.04.2016
     *
     * \tparam  XmlConfigReadFlag)  Type of the XML configuration read flag)
     *
     * \return  Xml configuration field value.
     */
    template <typename = typename enable_if<(f & XmlConfigReadFlag) == XmlConfigReadFlag>::type>
    operator T() const {
        return this->value;
    }

    /**
     * \fn  template < typename = typename XmlConfigValue::enable_if< (f & XmlConfigReadFlag) == XmlConfigReadFlag >::type > T operator()() const
     *
     * \brief   Get the xml configuration field value.
     *
     * \author  Pawel Iwaneczko
     * \date    06.04.2016
     *
     * \return  Xml configuration field value.
     */

    template <typename = typename enable_if<(f & XmlConfigReadFlag) == XmlConfigReadFlag>::type>
    T operator()() const {
        return this->value;
    }
    /**
     * \fn  template < typename = typename XmlConfigValue::enable_if< (f & XmlConfigWriteFlag) == XmlConfigWriteFlag >::type > void operator=(const T
     * &value)
     *
     * \brief   Set the xml configuration field value.
     *
     * \author  Pawel Iwaneczko
     * \date    06.04.2016
     *
     * \param   value   Xml configuration field value.
     */
    template <typename = typename enable_if<(f & XmlConfigWriteFlag) == XmlConfigWriteFlag>::type>
    void operator=(const T &value) {
        xml_attribute attr;
        XmlConfig::FindElement(*this, attr);
        this->value = value;
        InitValue(attr, false, this->value);
        XmlConfig::UpdateXmlFile();
    }
};

template <typename K, typename T, XmlConfigFlag f = XmlConfigReadFlag>
class XmlConfigMap : public XmlConfigGroup, public map<K, XmlConfigValue<T, f>> {
protected:
    /**
     * Insert pair to map
     */
    void insert(const pair<K, T> &p) {
        map<K, XmlConfigValue<T, f>>::insert(make_pair(p.first, XmlConfigValue<T, f>(to_string(p.first), p.second, this)));
    }

public:
    /**
     * \fn  XmlConfigMap::XmlConfigMap(const strifng &name);
     *
     * \brief   Constructor of xml configuration map.
     *
     * \author  Pawel Iwaneczko
     * \date    06.04.2016
     *
     * \param   name        The name of xml configuration elements map.
     */
    XmlConfigMap(const string &name) : XmlConfigGroup(name) {}
    /**
     * Get key value
     */
    T at(const K &key) const {
        return map<K, XmlConfigValue<T, f>>::at(key);
    }
    /**
     * Znalezienie elementu w kolekcji
     */
    bool find(const K &key) const {
        return map<K, XmlConfigValue<T, f>>::find(key) != end();
    }
};

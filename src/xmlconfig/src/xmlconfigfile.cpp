#include <iostream>
#include "XmlConfig.hpp"
using namespace std;

#if XML_CONFIG_NOT_DEFAULT_FILE_PATH
#define USE_MODULE_PATH 0
#else
#define USE_MODULE_PATH 1
#include <Windows.h>
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#endif

XmlConfig::XmlConfig() {
#if USE_MODULE_PATH
    char modulePath[MAX_PATH];
    GetModuleFileNameA((HINSTANCE)&__ImageBase, modulePath, MAX_PATH);
    filePath = modulePath;
    filePath.append(".xml");
    if (doc.load_file(filePath.c_str()).status != status_ok) {
        doc.reset();
    }
#endif
}

XmlConfig &XmlConfig::GetInstance() {
    static XmlConfig xml;
    return xml;
}

XmlConfig::~XmlConfig() {
    UpdateXmlFile();
}

void XmlConfig::UpdateXmlFile() {
    XmlConfig &xml = GetInstance();
    if (xml.filePath.empty()) {
#if USE_MODULE_PATH
        xml.doc.reset();
#endif
        return;
    }
    xml.doc.save_file(xml.filePath.c_str());
}
void XmlConfig::SetXMLFilePath(string filePath) {
    XmlConfig &xml = GetInstance();
    if (xml.filePath != filePath) {
        xml.filePath = filePath;
        xml_document doc;
        if (doc.load_file(filePath.c_str()).status == status_ok)
            xml.doc.load_file(filePath.c_str());
        else
            UpdateXmlFile();
    }
}
string XmlConfig::GetXMLFilePath() {
    XmlConfig &xml = GetInstance();
    return xml.filePath;
}
bool XmlConfig::FindElement(const XmlConfigElement &element, xml_attribute &attr) {
    XmlConfig &xml = XmlConfig::GetInstance();

    if (xml.doc.load_file(xml.filePath.c_str()).status != status_ok) {
#if USE_MODULE_PATH
        xml.doc.reset();
#endif
        return false;
    }

    XmlConfigGroup *group = element.parent;

    bool result = true;
    xml_node config = xml.doc.child("configuration");
    if (config.empty()) {
        config = xml.doc.append_child("configuration");
        result = false;
    }
    if (group != nullptr) {
        // element posiada grup�
        xml_node gr_node;
        result = false;
        for (xml_node &gr : config.children("group")) {
            if (gr.attribute("name").as_string() == group->name) {
                gr_node = gr;
                result = true;
                break;
            }
        }
        if (!result) {
            gr_node = config.append_child("group");
            gr_node.append_attribute("name") = group->name.c_str();
        }
        config = gr_node;
    }

    // wyszukiwanie ustawienia
    xml_node node;
    for (auto el : config.children("setting")) {
        if (el.attribute("name").as_string() == element.name) {
            node = el;
            break;
        }
    }
    // czy ustawienie istnieje
    if (node.empty()) {
        // nie - nale�y je utworzy�
        node = config.append_child("setting");
        node.append_attribute("name") = element.name.c_str();
        result = false;
    }

    // wyszukiwanie atrybutu warto�ci
    attr = node.attribute("value");
    if (attr.empty()) {
        // nie istnieje - tworzenie
        attr = node.append_attribute("value");
        result = false;
    }

    if (!result) UpdateXmlFile();

    assert(!attr.empty() && "attribute is empty");
    return result;
}

/**
 * \brief       Source of the XML configuration class
 * \file        XmlConfig.cpp
 * \author      Pawe³ Iwaneczko
 * \copyright   Copyright 2016 HTeam. All rights reserved.
 */
#include "XmlConfig.hpp"

XmlConfigElement::XmlConfigElement(const string & name, XmlConfigGroup *parent) : 
    name(name), parent(parent) {
    if (parent != nullptr) {
        parent->elements.insert(make_pair(name, this));
    }
}

string XmlConfigElement::GetName() const { 
    return name;
}

XmlConfigGroup::XmlConfigGroup(const string & name) : XmlConfigElement(name) {}
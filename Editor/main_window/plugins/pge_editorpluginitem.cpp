#include "pge_editorpluginitem.h"

PGE_EditorPluginItem::PGE_EditorPluginItem(const QString &pluginName, const QString &authorName, const QString &description, int pluginVersionNumber) :
    m_pluginName(pluginName),
    m_authorName(authorName),
    m_description(description),
    m_pluginVersionNum(pluginVersionNumber)
{}

QString PGE_EditorPluginItem::pluginName() const
{
    return m_pluginName;
}

QString PGE_EditorPluginItem::authorName() const
{
    return m_authorName;
}

int PGE_EditorPluginItem::pluginVersionNum() const
{
    return m_pluginVersionNum;
}

QString PGE_EditorPluginItem::description() const
{
    return m_description;
}

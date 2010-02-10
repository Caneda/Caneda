#ifndef SETTINGS_H
#define SETTINGS_H

#include <QMap>
#include <QVariant>

class QSettings;

struct VariantPair
{
    QVariant defaultValue;
    QVariant currentValue;

    VariantPair(const QVariant& def = QVariant(),
            const QVariant& cur = QVariant());
};

typedef QMap<QString, VariantPair> SettingsData;

struct Settings
{
    ~Settings();

    QVariant currentValue(const QString& key) const;
    QVariant defaultValue(const QString& key) const;

    void setCurrentValue(const QString& key, const QVariant& value);

    bool load(QSettings &settings);
    bool save(QSettings &settings);

    static Settings* instance();
private:
    SettingsData data;
    Settings();
};

#endif

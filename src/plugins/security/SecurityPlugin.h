#ifndef SECURITYPLUGIN_H
#define SECURITYPLUGIN_H

#include "IPlugin.h"

class SecurityPlugin : public IPlugin {
public:
    SecurityPlugin();
    ~SecurityPlugin() override;

    bool initialize() override;
    void cleanup() override;
    QString getName() const override;
    QString getVersion() const override;
    QString getDescription() const override;
    QVariant execute(const QString &function, const QVariantMap &params) override;

private:
    bool m_initialized;
};

#endif // SECURITYPLUGIN_H
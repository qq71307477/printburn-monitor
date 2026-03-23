#ifndef CLIENTPLUGIN_H
#define CLIENTPLUGIN_H

#include "IPlugin.h"

class ClientPlugin : public IPlugin {
public:
    ClientPlugin();
    ~ClientPlugin() override;

    bool initialize() override;
    void cleanup() override;
    QString getName() const override;
    QString getVersion() const override;
    QString getDescription() const override;
    QVariant execute(const QString &function, const QVariantMap &params) override;

private:
    bool m_initialized;
};

#endif // CLIENTPLUGIN_H
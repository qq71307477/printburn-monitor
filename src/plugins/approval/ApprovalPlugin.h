#ifndef APPROVALPLUGIN_H
#define APPROVALPLUGIN_H

#include "IPlugin.h"

class ApprovalPlugin : public IPlugin {
public:
    ApprovalPlugin();
    ~ApprovalPlugin() override;

    bool initialize() override;
    void cleanup() override;
    QString getName() const override;
    QString getVersion() const override;
    QString getDescription() const override;
    QVariant execute(const QString &function, const QVariantMap &params) override;

private:
    bool m_initialized;
};

#endif // APPROVALPLUGIN_H
#ifndef TASKPLUGIN_H
#define TASKPLUGIN_H

#include "IPlugin.h"

class TaskPlugin : public IPlugin {
public:
    TaskPlugin();
    ~TaskPlugin() override;

    bool initialize() override;
    void cleanup() override;
    QString getName() const override;
    QString getVersion() const override;
    QString getDescription() const override;
    QVariant execute(const QString &function, const QVariantMap &params) override;

private:
    bool m_initialized;
};

#endif // TASKPLUGIN_H
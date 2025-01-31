#pragma once

#include <QObject>
#include <QString>
#include "bzMagFileManager.h"
#include "Modeler.h"

class ProjectLoader : public QObject {
    Q_OBJECT
public:
    ProjectLoader(const QString& filename)
        : filename_(filename) {
    }

public slots:
    void load() {
        Modeler* modeler = Modeler::instance();

        try {
            BzmagFileManager file_manager;
            file_manager.loadFromFile(filename_);

            emit taskStarted("Loading file...");
            bool result = modeler->loadSavedData(file_manager, [this](int progress) {
                emit progressUpdated(progress);
            });

            emit taskCompleted(result);
        }
        catch (const std::exception& ex) {
            emit errorOccurred(ex.what());
        }
    }

signals:
    void taskStarted(const QString& task);
    void progressUpdated(int progress);
    void taskCompleted(bool success);
    void errorOccurred(const QString& error);

private:
    QString filename_;
};
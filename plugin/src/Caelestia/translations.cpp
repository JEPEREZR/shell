#include <QCoreApplication>
#include <QFileInfo>
#include <QLocale>
#include <QLoggingCategory>
#include <QStandardPaths>
#include <QString>
#include <QStringList>
#include <QTranslator>

Q_LOGGING_CATEGORY(logTranslations, "caelestia.translations")

namespace {

QStringList translationSearchPaths() {
    QStringList paths;
    paths << QStringLiteral("/usr/share/caelestia-shell/translations")
          << QStringLiteral("/usr/local/share/caelestia-shell/translations");

    const QStringList dataDirs = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    for (const QString& dir : dataDirs) {
        paths << dir + QStringLiteral("/caelestia-shell/translations");
    }
    return paths;
}

bool installFromFile(QCoreApplication* app, const QString& localeName) {
    const QString filename = QStringLiteral("caelestia_%1.qm").arg(localeName);
    for (const QString& dir : translationSearchPaths()) {
        const QString fullPath = dir + QLatin1Char('/') + filename;
        if (!QFileInfo::exists(fullPath)) continue;

        auto* translator = new QTranslator(app);
        if (translator->load(fullPath)) {
            app->installTranslator(translator);
            qCInfo(logTranslations) << "Loaded" << filename << "from" << dir;
            return true;
        }
        qCWarning(logTranslations) << "Found but failed to load" << fullPath;
        delete translator;
    }
    return false;
}

void installCaelestiaTranslator() {
    QCoreApplication* app = QCoreApplication::instance();
    if (!app) {
        qCWarning(logTranslations) << "No QCoreApplication instance; skipping translation install.";
        return;
    }

    // Apply system locale as Qt's default so QML's Qt.locale(),
    // Qt.formatDateTime() and QML's `locale:` bindings inherit it.
    // Without this, Qt defaults to QLocale::c() (en-US) for date
    // formatting even when LANG/LC_TIME point at es_CL.
    QLocale::setDefault(QLocale::system());

    // Install least-specific first so more-specific overrides end up on
    // top of the translator stack (installTranslator prepends).
    const QString sysName = QLocale::system().name();  // e.g. "es_CL"
    const qsizetype sep = sysName.indexOf(QLatin1Char('_'));

    QStringList order;
    if (sep > 0) order << sysName.left(sep);
    order << sysName;

    bool installed = false;
    for (const QString& candidate : std::as_const(order)) {
        if (installFromFile(app, candidate)) installed = true;
    }

    if (!installed) {
        qCDebug(logTranslations) << "No translation file matched system locale" << sysName;
    }
}

}  // namespace

Q_COREAPP_STARTUP_FUNCTION(installCaelestiaTranslator)

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QLocale>
#include <QLoggingCategory>
#include <QStandardPaths>
#include <QString>
#include <QStringList>
#include <QTranslator>

Q_LOGGING_CATEGORY(logTranslations, "caelestia.translations")

namespace {

const char* const kTranslationPrefix = "caelestia_";

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

bool tryLoadInto(QTranslator* translator, const QString& localeName) {
    const QStringList paths = translationSearchPaths();
    for (const QString& dir : paths) {
        if (translator->load(QStringLiteral("%1%2").arg(QString::fromLatin1(kTranslationPrefix), localeName), dir)) {
            qCInfo(logTranslations) << "Loaded translation" << localeName << "from" << dir;
            return true;
        }
    }
    return false;
}

void installCaelestiaTranslator() {
    QCoreApplication* app = QCoreApplication::instance();
    if (!app) {
        qCWarning(logTranslations) << "No QCoreApplication instance; skipping translation install.";
        return;
    }

    const QLocale system = QLocale::system();
    QStringList candidates = system.uiLanguages();
    for (QString& c : candidates) {
        c.replace(QLatin1Char('-'), QLatin1Char('_'));
    }

    QStringList tried;
    bool installedAny = false;

    auto attempt = [&](const QString& name) {
        if (name.isEmpty() || tried.contains(name)) return;
        tried << name;
        auto* t = new QTranslator(app);
        if (tryLoadInto(t, name)) {
            app->installTranslator(t);
            installedAny = true;
        } else {
            delete t;
        }
    };

    for (const QString& candidate : std::as_const(candidates)) {
        attempt(candidate);
        const qsizetype sep = candidate.indexOf(QLatin1Char('_'));
        if (sep > 0) {
            attempt(candidate.left(sep));
        }
    }

    if (!installedAny) {
        qCDebug(logTranslations) << "No translation file matched system locale" << system.name()
                                 << "; candidates tried:" << tried;
    }
}

}  // namespace

Q_COREAPP_STARTUP_FUNCTION(installCaelestiaTranslator)

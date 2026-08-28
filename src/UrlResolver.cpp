#include "UrlResolver.h"

#include <QHostAddress>
#include <QRegularExpression>

namespace {
const QRegularExpression kExplicitScheme(
    QStringLiteral(R"(^[A-Za-z][A-Za-z0-9+.-]*://)"));
const QRegularExpression kSpecialScheme(
    QStringLiteral(R"(^(?:about|data|file|mailto|qrc|view-source):)"),
    QRegularExpression::CaseInsensitiveOption);
const QRegularExpression kHostAndRemainder(
    QStringLiteral(R"(^([^/?#]+)(?:[/?#].*)?$)"));
const QRegularExpression kDomain(
    QStringLiteral(R"(^(?:[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?\.)+[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?$)"));
}

QUrl UrlResolver::resolve(const QString &input, const QString &searchUrlTemplate)
{
    const QString text = input.trimmed();
    if (text.isEmpty()) {
        return {};
    }

    if (kExplicitScheme.match(text).hasMatch() || kSpecialScheme.match(text).hasMatch()) {
        const QUrl url(text, QUrl::TolerantMode);
        if (url.isValid()) {
            return url;
        }
    }

    if (looksLikeAddress(text)) {
        return QUrl(QStringLiteral("https://") + text, QUrl::TolerantMode);
    }

    const QString encodedQuery = QString::fromLatin1(QUrl::toPercentEncoding(text));
    return QUrl(searchUrlTemplate.arg(encodedQuery));
}

bool UrlResolver::looksLikeAddress(const QString &input)
{
    if (input.contains(QRegularExpression(QStringLiteral(R"(\s)")))) {
        return false;
    }

    const auto match = kHostAndRemainder.match(input);
    if (!match.hasMatch()) {
        return false;
    }

    QString authority = match.captured(1);
    QString host = authority;
    QString port;

    if (authority.startsWith(u'[')) {
        const qsizetype closingBracket = authority.indexOf(u']');
        if (closingBracket < 0) return false;
        host = authority.mid(1, closingBracket - 1);
        if (closingBracket + 1 < authority.size()) {
            if (authority.at(closingBracket + 1) != u':') return false;
            port = authority.mid(closingBracket + 2);
        }
    } else {
        const qsizetype colon = authority.lastIndexOf(u':');
        if (colon >= 0) {
            host = authority.left(colon);
            port = authority.mid(colon + 1);
        }
    }

    if (!port.isEmpty()) {
        bool ok = false;
        const int portNumber = port.toInt(&ok);
        if (!ok || portNumber < 1 || portNumber > 65535) return false;
    } else if (authority.endsWith(u':')) {
        return false;
    }

    if (host.compare(QStringLiteral("localhost"), Qt::CaseInsensitive) == 0) {
        return true;
    }

    QHostAddress address;
    if (address.setAddress(host)) {
        return true;
    }

    return kDomain.match(host).hasMatch();
}

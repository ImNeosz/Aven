#pragma once

#include <QString>
#include <QUrl>

class UrlResolver final
{
public:
    [[nodiscard]] static QUrl resolve(const QString &input, const QString &searchUrlTemplate);

private:
    [[nodiscard]] static bool looksLikeAddress(const QString &input);
};

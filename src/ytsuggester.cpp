/* $BEGIN_LICENSE

This file is part of Minitube.
Copyright 2009, Flavio Tordini <flavio.tordini@gmail.com>

Minitube is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Minitube is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Minitube.  If not, see <http://www.gnu.org/licenses/>.

$END_LICENSE */

#include "ytsuggester.h"
#include "networkaccess.h"

namespace The {
NetworkAccess* http();
}

YTSuggester::YTSuggester(QObject *parent) : Suggester(parent) {

}

void YTSuggester::suggest(const QString &query) {
    if (query.startsWith("http")) return;

#if QT_VERSION >= 0x040800
    QString locale = QLocale::system().uiLanguages().first();
#else
    QString locale = QLocale::system().name().replace("_", "-");
#endif

    // case for system locales such as "C"
    if (locale.length() < 2) {
        locale = "en-US";
    }

    QString url =
            QString("https://suggestqueries.google.com/complete/search?ds=yt&output=toolbar&hl=%1&q=%2")
            .arg(locale, query);

    QObject *reply = The::http()->get(url);
    connect(reply, SIGNAL(data(QByteArray)), SLOT(handleNetworkData(QByteArray)));
}

void YTSuggester::handleNetworkData(QByteArray response) {
    QList<Suggestion*> suggestions;
    QXmlStreamReader xml(response);
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == QLatin1String("suggestion")) {
                QStringRef str = xml.attributes().value("data");
                QString value = str.toString();
                suggestions << new Suggestion(value);
            }
        }
    }
    emit ready(suggestions);
}

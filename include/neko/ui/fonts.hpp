#pragma once

#include <utility>

#include <QtGui/QFont>

namespace neko::ui {

    std::pair<QFont, QFont> computeTitleFonts(const QFont &base, qreal h1Scale = 1.5, qreal h2Scale = 1.2){
        QFont h1Font = base;
        h1Font.setPointSizeF(base.pointSizeF() * h1Scale);
        h1Font.setBold(true);

        QFont h2Font = base;
        h2Font.setPointSizeF(base.pointSizeF() * h2Scale);
        h2Font.setItalic(true);

        return {h1Font, h2Font};
    }

} // namespace neko::ui
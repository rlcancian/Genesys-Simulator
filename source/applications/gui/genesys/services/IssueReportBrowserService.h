#ifndef ISSUEREPORTBROWSERSERVICE_H
#define ISSUEREPORTBROWSERSERVICE_H

#include "IssueReportRelayService.h"

#include <QUrl>
#include <QUrlQuery>

/** Constructs the public GitHub issue form URL. Never includes runtime diagnostics. */
class IssueReportBrowserService {
public:
    enum class Prefill { Complete, BodyRequiresPaste, AllRequiresPaste };

    struct Link {
        QUrl url;
        Prefill prefill = Prefill::Complete;
    };

    static QString body(const IssueReportDraft& draft) {
        return QStringLiteral("**Type:** %1\n\n%2")
            .arg(draft.categoryLabel, draft.summary);
    }

    static Link build(const IssueReportDraft& draft, qsizetype maxEncodedBytes = 1800) {
        const QUrl base(QStringLiteral(
            "https://github.com/rlcancian/Genesys-Simulator/issues/new"));

        QUrlQuery query;
        query.addQueryItem(QStringLiteral("title"), draft.title);
        query.addQueryItem(QStringLiteral("body"), body(draft));
        QUrl full(base);
        full.setQuery(query);
        if (full.toEncoded().size() <= maxEncodedBytes) {
            return {full, Prefill::Complete};
        }

        QUrlQuery titleOnly;
        titleOnly.addQueryItem(QStringLiteral("title"), draft.title);
        QUrl partial(base);
        partial.setQuery(titleOnly);
        if (partial.toEncoded().size() <= maxEncodedBytes) {
            return {partial, Prefill::BodyRequiresPaste};
        }
        return {base, Prefill::AllRequiresPaste};
    }
};

#endif // ISSUEREPORTBROWSERSERVICE_H

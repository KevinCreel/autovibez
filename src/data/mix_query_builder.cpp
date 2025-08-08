#include "mix_query_builder.hpp"

#include <sstream>

namespace AutoVibez {
namespace Data {

MixQueryBuilder& MixQueryBuilder::select() {
    query_parts_ = "SELECT * FROM mixes";
    return *this;
}

MixQueryBuilder& MixQueryBuilder::whereNotDeleted() {
    addWhereCondition("is_deleted = 0");
    return *this;
}

MixQueryBuilder& MixQueryBuilder::whereGenre() {
    addWhereCondition("genre COLLATE NOCASE = ? COLLATE NOCASE");
    parameter_count_++;
    return *this;
}

MixQueryBuilder& MixQueryBuilder::whereArtist() {
    addWhereCondition("artist = ?");
    parameter_count_++;
    return *this;
}

MixQueryBuilder& MixQueryBuilder::whereId() {
    addWhereCondition("id = ?");
    parameter_count_++;
    return *this;
}

MixQueryBuilder& MixQueryBuilder::whereNotId() {
    addWhereCondition("id != ?");
    parameter_count_++;
    return *this;
}

MixQueryBuilder& MixQueryBuilder::whereFavorites() {
    addWhereCondition("is_favorite = 1");
    return *this;
}

MixQueryBuilder& MixQueryBuilder::whereDownloaded() {
    addWhereCondition("local_path IS NOT NULL AND local_path != ''");
    return *this;
}

MixQueryBuilder& MixQueryBuilder::whereHasBeenPlayed() {
    addWhereCondition("last_played IS NOT NULL");
    return *this;
}

MixQueryBuilder& MixQueryBuilder::orderBy(OrderBy order, bool ascending) {
    std::string direction = ascending ? "ASC" : "DESC";

    switch (order) {
        case OrderBy::Title:
            order_clause_ = "ORDER BY title " + direction;
            break;
        case OrderBy::Artist:
            order_clause_ = "ORDER BY artist " + direction + ", title " + direction;
            break;
        case OrderBy::Genre:
            order_clause_ = "ORDER BY genre " + direction + ", title " + direction;
            break;
        case OrderBy::LastPlayed:
            order_clause_ = "ORDER BY last_played " + direction;
            break;
        case OrderBy::PlayCount:
            order_clause_ = "ORDER BY play_count " + direction;
            break;
        case OrderBy::DateAdded:
            order_clause_ = "ORDER BY date_added " + direction;
            break;
        case OrderBy::Random:
            order_clause_ = "ORDER BY RANDOM()";
            break;
        case OrderBy::None:
        default:
            order_clause_.clear();
            break;
    }

    return *this;
}

MixQueryBuilder& MixQueryBuilder::limit(int limit_value) {
    if (limit_value > 0) {
        limit_clause_ = "LIMIT " + std::to_string(limit_value);
    }
    return *this;
}

std::string MixQueryBuilder::build() const {
    std::ostringstream oss;
    oss << query_parts_;

    std::string where_clause = buildWhereClause();
    if (!where_clause.empty()) {
        oss << " " << where_clause;
    }

    if (!order_clause_.empty()) {
        oss << " " << order_clause_;
    }

    if (!limit_clause_.empty()) {
        oss << " " << limit_clause_;
    }

    return oss.str();
}

int MixQueryBuilder::getParameterCount() const {
    return parameter_count_;
}

MixQueryBuilder& MixQueryBuilder::reset() {
    query_parts_.clear();
    where_conditions_.clear();
    order_clause_.clear();
    limit_clause_.clear();
    parameter_count_ = 0;
    return *this;
}

std::string MixQueryBuilder::buildQuery(const SelectionCriteria& criteria, OrderBy order) {
    MixQueryBuilder builder;
    builder.select();

    // Always exclude deleted unless explicitly requested
    if (!criteria.include_deleted) {
        builder.whereNotDeleted();
    }

    // Add filters based on criteria
    if (!criteria.genre.empty()) {
        builder.whereGenre();
    }

    if (!criteria.artist.empty()) {
        builder.whereArtist();
    }

    if (!criteria.exclude_mix_id.empty()) {
        builder.whereNotId();
    }

    if (criteria.favorites_only) {
        builder.whereFavorites();
    }

    if (criteria.downloaded_only) {
        builder.whereDownloaded();
    }

    builder.orderBy(order);

    if (criteria.limit > 0) {
        builder.limit(criteria.limit);
    }

    return builder.build();
}

void MixQueryBuilder::addWhereCondition(const std::string& condition) {
    where_conditions_.push_back(condition);
}

std::string MixQueryBuilder::buildWhereClause() const {
    if (where_conditions_.empty()) {
        return "";
    }

    std::ostringstream oss;
    oss << "WHERE ";

    for (size_t i = 0; i < where_conditions_.size(); ++i) {
        if (i > 0) {
            oss << " AND ";
        }
        oss << where_conditions_[i];
    }

    return oss.str();
}

}  // namespace Data
}  // namespace AutoVibez

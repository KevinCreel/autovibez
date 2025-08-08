#pragma once

#include <string>
#include <vector>

namespace AutoVibez {
namespace Data {

/**
 * @brief Selection criteria for mix queries
 */
struct SelectionCriteria {
    std::string genre;
    std::string artist;
    std::string exclude_mix_id;
    bool favorites_only = false;
    bool downloaded_only = false;
    bool include_deleted = false;
    int limit = 0;  // 0 means no limit

    SelectionCriteria() = default;
};

/**
 * @brief Ordering options for queries
 */
enum class OrderBy { None, Title, Artist, Genre, LastPlayed, PlayCount, DateAdded, Random };

/**
 * @brief Builder pattern for constructing SQL queries for Mix operations
 *
 * This class provides a fluent interface for building complex SQL queries
 * while maintaining type safety and preventing SQL injection.
 */
class MixQueryBuilder {
public:
    /**
     * @brief Start building a SELECT query
     * @return Reference to this builder for chaining
     */
    MixQueryBuilder& select();

    /**
     * @brief Add WHERE clause for non-deleted mixes
     * @return Reference to this builder for chaining
     */
    MixQueryBuilder& whereNotDeleted();

    /**
     * @brief Add WHERE clause for specific genre
     * @return Reference to this builder for chaining
     */
    MixQueryBuilder& whereGenre();

    /**
     * @brief Add WHERE clause for specific artist
     * @return Reference to this builder for chaining
     */
    MixQueryBuilder& whereArtist();

    /**
     * @brief Add WHERE clause for specific mix ID
     * @return Reference to this builder for chaining
     */
    MixQueryBuilder& whereId();

    /**
     * @brief Add WHERE clause to exclude specific mix ID
     * @return Reference to this builder for chaining
     */
    MixQueryBuilder& whereNotId();

    /**
     * @brief Add WHERE clause for favorites only
     * @return Reference to this builder for chaining
     */
    MixQueryBuilder& whereFavorites();

    /**
     * @brief Add WHERE clause for downloaded mixes only
     * @return Reference to this builder for chaining
     */
    MixQueryBuilder& whereDownloaded();

    /**
     * @brief Add WHERE clause for mixes with last_played not null
     * @return Reference to this builder for chaining
     */
    MixQueryBuilder& whereHasBeenPlayed();

    /**
     * @brief Add ORDER BY clause
     * @param order Ordering option
     * @param ascending True for ASC, false for DESC
     * @return Reference to this builder for chaining
     */
    MixQueryBuilder& orderBy(OrderBy order, bool ascending = true);

    /**
     * @brief Add LIMIT clause
     * @param limit Maximum number of results
     * @return Reference to this builder for chaining
     */
    MixQueryBuilder& limit(int limit);

    /**
     * @brief Build the final SQL query string
     * @return Complete SQL query
     */
    std::string build() const;

    /**
     * @brief Get the number of parameter placeholders in the query
     * @return Number of ? placeholders
     */
    int getParameterCount() const;

    /**
     * @brief Reset the builder to start fresh
     * @return Reference to this builder
     */
    MixQueryBuilder& reset();

    /**
     * @brief Build a query based on selection criteria
     * @param criteria Selection criteria
     * @param order Ordering option
     * @return Complete SQL query
     */
    static std::string buildQuery(const SelectionCriteria& criteria, OrderBy order = OrderBy::Title);

private:
    std::string query_parts_;
    std::vector<std::string> where_conditions_;
    std::string order_clause_;
    std::string limit_clause_;
    int parameter_count_;

    /**
     * @brief Add a WHERE condition
     * @param condition SQL condition with ? placeholders
     */
    void addWhereCondition(const std::string& condition);

    /**
     * @brief Build the WHERE clause from accumulated conditions
     * @return WHERE clause string
     */
    std::string buildWhereClause() const;
};

}  // namespace Data
}  // namespace AutoVibez

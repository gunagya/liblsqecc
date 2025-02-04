#ifndef LSQECC_ROUTER_HPP
#define LSQECC_ROUTER_HPP

#include <lsqecc/patches/sparse_slice.hpp>
#include <lsqecc/patches/slice.hpp>

#include <unordered_map>

namespace lsqecc {

enum class GraphSearchProvider
{
    Boost,
    Djikstra,
    AStar
};


struct Router {
    virtual std::optional<RoutingRegion> find_routing_ancilla(
                const Slice& slice,
                PatchId source,
                PauliOperator source_op,
                PatchId target,
                PauliOperator target_op
            ) const = 0;

    virtual void set_graph_search_provider(GraphSearchProvider graph_search_provider) = 0;
    virtual void set_EDPC() = 0;
    virtual bool get_EDPC() = 0;

    virtual ~Router(){};
};


struct CustomDPRouter : public Router
{
    std::optional<RoutingRegion> find_routing_ancilla(
            const Slice& slice,
            PatchId source,
            PauliOperator source_op,
            PatchId target,
            PauliOperator target_op
    ) const override;

    void set_graph_search_provider(GraphSearchProvider graph_search_provider) override {
        graph_search_provider_ = graph_search_provider;
    };

    void set_EDPC() override {
        if (graph_search_provider_ == GraphSearchProvider::Boost) {
            throw std::logic_error("EDPC not implemented for Boost.");
        }
        else {
            EDPC = 1;
        }
    }

    bool get_EDPC() override {return EDPC;}

private:
    GraphSearchProvider graph_search_provider_ = GraphSearchProvider::Djikstra;
    bool EDPC = 0;

};

/**
 * Assumes paths are always clear. If a new patch appears on a previous route, route will go right over it.
 * TODO: Fix that by checking that the path is clear. Will require caching occupied cells
 *
 * Also assumes that all patches are LayoutHelpers::basic_square_patch
 */
struct CachedRouter : public Router
{

    std::optional<RoutingRegion> find_routing_ancilla(
            const Slice& slice,
            PatchId source,
            PauliOperator source_op,
            PatchId target,
            PauliOperator target_op
    ) const override;


    void set_graph_search_provider(GraphSearchProvider graph_search_provider) override {
        router_impl_.set_graph_search_provider(graph_search_provider);
    };

    void set_EDPC() override {throw std::logic_error("EDPC not implemented for cached router.");}
    bool get_EDPC() override {return 0;}

    struct PathIdentifier {
        Cell source_cell;
        PauliOperator source_op;
        Cell target_cell;
        PauliOperator target_op;

        bool operator==(const PathIdentifier&) const = default;
        struct hash
        {
            size_t operator()( const PathIdentifier& x ) const;
        };
    };


private:

    CustomDPRouter router_impl_;
    mutable std::unordered_map<PathIdentifier, RoutingRegion, PathIdentifier::hash> cached_routes_;
};
}


#endif //LSQECC_ROUTER_HPP

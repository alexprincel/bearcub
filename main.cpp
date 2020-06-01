#include <iostream>
#include <string>
#include <vector>
#include <boost/graph/adjacency_list.hpp>

// sequence of rules (Pipeline)
// - folder rules
// - file rules
// - content rules
// - meta rules
// Queryset object
// - List of files
// - Data ? Stored in-mem
// read data files (csv)
// aggregate multiple files
// - pattern match files
// - accomodate different file structures
// - - missing columns
// - - additional columns


// Store file metadata
struct File {

};

// Store and manage data and metadata associated with a specific pipeline execution
class QuerySet {
    public:
    private:

};

// Execute a set of extraction, transformation or output instructions on a query set
class Rule {
    public:
        virtual void ExecuteRule() = 0;
};

class TransformationRule : public Rule {
    public:
        void ExecuteRule() {
            std::cout << "Transformation Rule !";
        }

        friend std::ostream& operator<<(std::ostream& os, TransformationRule& er) {
            os << "Transformation rule !";
            return os;
        }
};

class ExtractionRule : public Rule {
    public:
        void ExecuteRule() {
            std::cout << "Extraction rule !";
        }

        friend std::ostream& operator<<(std::ostream& os, ExtractionRule& er) {
            os << "Extraction rule !";
            return os;
        }
};

// Store and manage data collection, transformation and output rules
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, std::shared_ptr<Rule>> BiDirectedGraph;
typedef boost::optional<unsigned int> opt_uint;

class Pipeline {
public:
    void AddRule(std::shared_ptr<Rule> new_rule, opt_uint input_rule_id, opt_uint output_rule_id) {
        BiDirectedGraph::vertex_descriptor vd = boost::add_vertex(new_rule, rule_graph);
        if(input_rule_id) {
            boost::add_edge(*input_rule_id, vd, rule_graph);
        }
        if(output_rule_id) {
            boost::add_edge(vd, *output_rule_id, rule_graph);
        }
    }

    std::vector<std::shared_ptr<Rule>> GetRootRules() const {
        std::vector<std::shared_ptr<Rule>> root_rules;
        for(auto vd : boost::make_iterator_range(boost::vertices(rule_graph))) {
            auto in_edges = boost::make_iterator_range(boost::in_edges(vd, rule_graph));
            if(in_edges.size() == 0) {
                root_rules.push_back(rule_graph[vd]);
            }
        }
        return root_rules;
    }

    void ExecuteRules() {
        for(auto vd : boost::make_iterator_range(boost::vertices(rule_graph))) {
            rule_graph[vd]->ExecuteRule();
        }
    }

    void PrintPipeline() {
        std::cout << "****************************************\n";
        std::cout << "Network has " << boost::num_vertices(rule_graph) 
                << " vertices and " << boost::num_edges(rule_graph) << " edges\n";
        std::cout << "Vertex list:\n";
        for(auto ed : boost::make_iterator_range(boost::edges(rule_graph))) {
            /*std::cout << "(" << (*rule_graph[boost::source(ed, rule_graph)])
                    << ", " << (*rule_graph[boost::target(ed, rule_graph)]) << ")\n";*/
        }
        for(auto in_less_vertex : GetRootRules()) {
            in_less_vertex->ExecuteRule();
        }
        std::cout << "****************************************\n";
    }
private:
    BiDirectedGraph rule_graph;
};



int main(int argc, char** argv) {
    Pipeline data_pipeline;
    data_pipeline.AddRule(std::make_shared<TransformationRule>(), opt_uint(), opt_uint());
    data_pipeline.AddRule(std::make_shared<ExtractionRule>(), opt_uint(), 0);
    data_pipeline.PrintPipeline();
    return 0;
}
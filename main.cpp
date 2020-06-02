#include <iostream>
#include <string>
#include <vector>
#include <boost/graph/adjacency_list.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

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
        virtual bool IsReadyToExecute() = 0;
};

class TransformationRule : public Rule {
    public:
        void ExecuteRule() {
            std::cout << "Transformation Rule !";
        }

        bool IsReadyToExecute() {
            boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
            return true;
        }
};

class ExtractionRule : public Rule {
    public:
        void ExecuteRule() {
            std::cout << "Extraction rule !";
        }

        bool IsReadyToExecute() {
            return true;
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
        // Pour l'instant, traverse dans n'importe quelle direction
        // OBJECTIF : Partir des nodes du début et aller vers l'avant:
        // PROBLÈMES À RÉGLER:
        // - Plusieurs nodes doivent pouvoir s'exécuter en même temps (multithreading / parallel computing)
        // - Plusieurs nodes nécessaires à un node (ne peut commencer tant que nodes d'avant ont terminé leur exécution)
        //      Ajout d'une propriété sur chaque node pour indiquer leur état ?
        // - Possible récurrence des relations
        //      Check de récurrence et ignorer ?
        
        // for(auto vd : boost::make_iterator_range(boost::vertices(rule_graph))) {
        //     rule_graph[vd]->ExecuteRule();
        // }

        for(auto rule : this->GetRootRules()) {
            boost::thread child_thread{boost::bind(Rule::ExecuteRule, rule)};
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
        std::cout << "\n****************************************\n";
    }
private:
    BiDirectedGraph rule_graph;
};



int main(int argc, char** argv) {
    Pipeline data_pipeline;

    // End node
    data_pipeline.AddRule(std::make_shared<ExtractionRule>(), opt_uint(), opt_uint());

    // Middle node
    data_pipeline.AddRule(std::make_shared<TransformationRule>(), opt_uint(), 0);
    
    // Beginning nodes
    data_pipeline.AddRule(std::make_shared<TransformationRule>(), opt_uint(), 1);
    data_pipeline.AddRule(std::make_shared<ExtractionRule>(), opt_uint(), 1);

    data_pipeline.ExecuteRules();

    //data_pipeline.PrintPipeline();
    
    return 0;
}
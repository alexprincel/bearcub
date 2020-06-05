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
            boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
            std::cout << "\nTransformation Rule !" << std::flush;
        }

        bool IsReadyToExecute() {
            
            return true;
        }
};

class ExtractionRule : public Rule {
    public:
        void ExecuteRule() {
            std::cout << "\nExtraction rule !" << std::flush;
        }

        bool IsReadyToExecute() {
            return true;
        }
};


// Store and manage data collection, transformation and output rules
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, std::shared_ptr<Rule>> BiDirectedGraph;
typedef boost::optional<unsigned int> opt_uint;
typedef std::shared_ptr<boost::thread> thread_ptr;
typedef std::shared_ptr<Rule> rule_ptr;

// bool ThreadHasFinished(thread_ptr t) {
//     // std::cout << "\nChecking thread...";
//     bool retval = t->timed_join(boost::posix_time::milliseconds(0));
//     // if(retval) std::cout << "\nThread has finished";
//     return retval;
// }

class Pipeline {
public:
    void AddRule(rule_ptr new_rule, opt_uint input_rule_id, opt_uint output_rule_id) {
        BiDirectedGraph::vertex_descriptor vd = boost::add_vertex(new_rule, rule_graph);
        if(input_rule_id) {
            boost::add_edge(*input_rule_id, vd, rule_graph);
        }
        if(output_rule_id) {
            boost::add_edge(vd, *output_rule_id, rule_graph);
        }
    }

    std::vector<rule_ptr> GetRootRules() const {
        std::vector<rule_ptr> root_rules;
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

        std::vector<thread_ptr> thread_vec;
        // std::cout << "\nNetwork has " << this->GetRootRules().size() << " root rules.";
        for(auto rule : this->GetRootRules()) {
            thread_ptr child_thread = std::make_shared<boost::thread>(&Rule::ExecuteRule, rule);
            std::cout << "\nThread id " << child_thread->get_id() << " created !" << std::flush;
            thread_vec.push_back(child_thread);
        }

        while(true) {
            // std::cout << "\nThread iteration...";
            // std::cout << "\nthread_vec.size() == " << thread_vec.size();
            if(thread_vec.size() == 0) break;

            thread_vec.erase(
                std::remove_if(
                    thread_vec.begin(), 
                    thread_vec.end(), 
                    [](thread_ptr thread) -> bool { 
                        return thread->timed_join(boost::posix_time::milliseconds(0)) == 0; 
                    }
                ),
                thread_vec.end()
            );

            boost::this_thread::sleep(boost::posix_time::milliseconds(100));
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
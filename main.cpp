#include <iostream>
#include <string>
#include <vector>
#include <boost/graph/adjacency_list.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

// sequence of Tasks (Pipeline)
// - folder Tasks
// - file Tasks
// - content Tasks
// - meta Tasks
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
class Task {
    public:
        virtual void ExecuteTask() = 0;
        virtual bool IsReadyToExecute() = 0;
};

class TransformationTask : public Task {
    public:
        void ExecuteTask() {
            boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
            std::cout << "\nTransformation Task !" << std::flush;
        }

        bool IsReadyToExecute() {
            
            return true;
        }
};

class ExtractionTask : public Task {
    public:
        void ExecuteTask() {
            std::cout << "\nExtraction Task !" << std::flush;
        }

        bool IsReadyToExecute() {
            return true;
        }
};


// Store and manage data collection, transformation and output Tasks
typedef std::shared_ptr<boost::thread> thread_ptr;
typedef std::shared_ptr<Task> task_ptr;
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, task_ptr, thread_ptr> BiDirectedGraph;
typedef boost::optional<unsigned int> opt_uint;

bool ThreadHasFinished(std::pair<task_ptr, thread_ptr>& task_process_pair) {
    bool retval = task_process_pair.second->timed_join(boost::posix_time::milliseconds(0));
    return retval;
}

class Pipeline {
public:
    void AddTask(task_ptr new_Task, opt_uint input_Task_id, opt_uint output_Task_id) {
        BiDirectedGraph::vertex_descriptor vd = boost::add_vertex(new_Task, task_graph);
        if(input_Task_id) {
            boost::add_edge(*input_Task_id, vd, task_graph);
        }
        if(output_Task_id) {
            boost::add_edge(vd, *output_Task_id, task_graph);
        }
    }

    std::vector<boost::graph_traits<BiDirectedGraph>::vertex_descriptor> GetRootTasks() const {
        std::vector<boost::graph_traits<BiDirectedGraph>::vertex_descriptor> root_Tasks;
        for(auto vd : boost::make_iterator_range(boost::vertices(task_graph))) {
            auto in_edges = boost::make_iterator_range(boost::in_edges(vd, task_graph));
            if(in_edges.size() == 0) {
                root_Tasks.push_back(vd);
            }
        }
        return root_Tasks;
    }

    void ExecuteTasks() {
        // Pour l'instant, traverse dans n'importe quelle direction
        // OBJECTIF : Partir des nodes du début et aller vers l'avant:
        // PROBLÈMES À RÉGLER:
        // - Plusieurs nodes doivent pouvoir s'exécuter en même temps (multithreading / parallel computing)
        // - Plusieurs nodes nécessaires à un node (ne peut commencer tant que nodes d'avant ont terminé leur exécution)
        //      Ajout d'une propriété sur chaque node pour indiquer leur état ?
        // - Possible récurrence des relations
        //      Check de récurrence et ignorer ?
        
        // for(auto vd : boost::make_iterator_range(boost::vertices(task_graph))) {
        //     task_graph[vd]->ExecuteTask();
        // }

        std::vector<std::pair<task_ptr, thread_ptr>> task_execution_list;

        std::vector<thread_ptr> thread_vec;
        // std::cout << "\nNetwork has " << this->GetRootTasks().size() << " root Tasks.";
        for(auto Task : this->GetRootTasks()) {
            thread_ptr child_thread = std::make_shared<boost::thread>(&Task::ExecuteTask, task_graph[Task]);
            std::cout << "\nThread id " << child_thread->get_id() << " created !" << std::flush;
            task_execution_list.push_back(std::make_pair(task_graph[Task], child_thread));
        }

        while(true) {
            if(task_execution_list.size() == 0) break;

            task_execution_list.erase(
                std::remove_if(
                    task_execution_list.begin(),
                    task_execution_list.end(),
                    ThreadHasFinished
                ),
                task_execution_list.end()
            );

            boost::this_thread::sleep(boost::posix_time::milliseconds(100));
        }
    }

    void PrintPipeline() {
        std::cout << "****************************************\n";
        std::cout << "Network has " << boost::num_vertices(task_graph) 
                << " vertices and " << boost::num_edges(task_graph) << " edges\n";
        std::cout << "Vertex list:\n";
        for(auto ed : boost::make_iterator_range(boost::edges(task_graph))) {
            /*std::cout << "(" << (*task_graph[boost::source(ed, task_graph)])
                    << ", " << (*task_graph[boost::target(ed, task_graph)]) << ")\n";*/
        }
        for(auto in_less_vertex : GetRootTasks()) {
            //in_less_vertex->ExecuteTask();
        }
        std::cout << "\n****************************************\n";
    }
private:
    std::vector<task_ptr> GetNextTasks(BiDirectedGraph::vertex_descriptor vd) {
        std::vector<task_ptr> task_list;
        for(auto out_edge : boost::make_iterator_range(boost::out_edges(vd, task_graph))) {
            task_list.push_back(task_graph[boost::target(out_edge, task_graph)]);
        }
        return task_list;
    }

    BiDirectedGraph task_graph;
};



int main(int argc, char** argv) {
    Pipeline data_pipeline;

    // End node
    data_pipeline.AddTask(std::make_shared<ExtractionTask>(), opt_uint(), opt_uint());

    // Middle node
    data_pipeline.AddTask(std::make_shared<TransformationTask>(), opt_uint(), 0);
    
    // Beginning nodes
    data_pipeline.AddTask(std::make_shared<TransformationTask>(), opt_uint(), 1);
    data_pipeline.AddTask(std::make_shared<ExtractionTask>(), opt_uint(), 1);

    data_pipeline.ExecuteTasks();
    
    //data_pipeline.PrintPipeline();
    
    return 0;
}
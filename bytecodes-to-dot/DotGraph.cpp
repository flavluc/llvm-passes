#include <vector>
#include <map> 

using namespace std;

namespace dot {
  class Graph {

    private:
      string title;
      string result = "";
      map<string, vector<string>> adj;
      map<string, string> labels;

      string genDotNode(string &node) {
          string label = this->labels[node];

          return node+" [shape=record, label=\"{"+label+"}\"];\n";
      }

      string genDotEdge(string &n1, string &n2) {
        return n1+" -> "+n2+";\n";
      }


    public:
      Graph (string _title) : title(_title) {}

      void node(string node, string label) {
        this->adj[node] = vector<string>();
        this->labels[node] = label;
      }

      void edge(string n1, string n2) {
        this->adj[n1].push_back(n2);
      }

      string genDot() {

        if (this->result != "")
          return this->result;

        this->result = "digraph \"" + this->title + "\" {\n";

        for (pair<string, vector<string>> entry : adj) {
          string node = entry.first;
          this->result += this->genDotNode(node);

          vector<string> successors = entry.second;
          for(string &succ : successors)
            this->result += this->genDotEdge(node, succ);
        }

        return this->result + "}";
      }
  };
}
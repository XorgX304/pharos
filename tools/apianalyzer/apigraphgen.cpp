// Copyright 2015, 2016 Carnegie Mellon University.  See LICENSE file for terms.

#include <rose.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/connected_components.hpp>

#include <libpharos/pdg.hpp>
#include <libpharos/misc.hpp>
#include <libpharos/descriptors.hpp>
#include <libpharos/masm.hpp>
#include <libpharos/defuse.hpp>
#include <libpharos/sptrack.hpp>
#include <libpharos/options.hpp>

#include <libpharos/apigraph.hpp>
#include <libpharos/apisig.hpp>

using namespace pharos;

// The global CERT message facility.  This may change.
Sawyer::Message::Facility glog("APIG");

const std::string VERSION  = "1.01";

// add extra command line options for apianalyzer
ProgOptDesc apianalyzer_options() {
  namespace po = boost::program_options;

  std::string version_string = "ApiGraphGen v" + VERSION + " options";
  ProgOptDesc apiopt(version_string.c_str());

  apiopt.add_options()
    ("graphviz,G", po::value<std::string>(), "specify the graphviz output file");

  return apiopt;
}

// Need to swtich this to use pharos_main.  Or better, make this functionality
// simply be a cmd line arg to apianalyzer instead of a separate exec.
int main(int argc, char* argv[]) {

  ProgOptDesc apiod = apianalyzer_options();
  ProgOptDesc csod = cert_standard_options();
  apiod.add(csod);

  ProgOptVarMap vm = parse_cert_options(argc, argv, apiod);

  std::ostringstream file_name;
  file_name << vm["graphviz"].as<std::string>();
  std::string gv_file = file_name.str();

  // end configuration, begin analysis

  // Find calls, functions, and imports.
  DescriptorSet ds(&vm);
  // Resolve imports, load API data, etc.
  ds.resolve_imports();

  // Build PDGs
  BottomUpAnalyzer bua(&ds, vm);
  bua.analyze();

  // end analysis, start graph generation

  GINFO << "Starting API Graph generation" << LEND;

  ApiGraph graph;
  size_t num_components = graph.Build();

  OINFO << "Completed API Graph generation with " << num_components << " components" << LEND;
  OINFO << "Writing graphviz to " << gv_file << LEND;

  std::ofstream graphviz_file(gv_file.c_str());

  graph.GenerateGraphViz(graphviz_file);

  OINFO << "ApiGraphGen complete." << LEND;
  global_rops.reset();

  return 0;
}

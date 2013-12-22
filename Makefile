all:
	g++ -g -o agent agent.cc catalog.cc resource.cc -lcurl -lc++ -Irapidjson

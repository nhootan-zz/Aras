# Aras

This is the "official" distribution of Aras.

## License

Aras is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

Aras is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <https://www.gnu.org/licenses/>.

## Build Instructions

Aras is built on top of Fast Downward (FD) code base: http://www.fast-downward.org.
Hence, the building process is the same:

```
git clone https://github.com/nhootan/Aras.git
cd Aras/src
./build_all
```

Parts of FD are implemented in C++, and parts are implemented
in Python. You will need a Python interpreter, version 2.7  to
run it.

## Running Aras

To run the algorithm once to find a single plan, use:
```
cd Aras/src
./plan domainfile problemfile aras_config
```

Typical Aras configuration:
```
--postprocessor "aras()" --input-plan-file input_file --plan-file output_file
```

Example: 
```
./plan Elevators/domain.pddl Elevators/p07.pddl --postprocessor "aras()" --input-plan-file aras_input --plan-file aras_output
```

To get a description of the available parameters for Aras use 
```
src/downward-1 --help aras
```

Unless it is necessary, please control the memory and time limits (ulimit) from outside the program:
the memory_limit implemented in aras does not work accurately. 

For domains with conditional effects please set the parameter reg_graph to false.
Example: 
```
./plan Elevators/domain.pddl Elevators/p07.pddl --postprocessor "aras(reg_graph=false)" --input-plan-file aras_input --plan-file aras_output
```

## Questions and Feedback

Email (in domain gmail.com): nhootan

## References

1. Hootan Nakhost. Random Walk Planning: Theory, Practice, and Application. PhD thesis, University of Alberta, 2013.
2. Hootan Nakhost and Martin M&uuml;ller. Action Elimination and Plan Neighborhood Graph Search: Two Algorithms for Plan Improvement. Proceedings of the 20th International Conference on Automated Planning and Scheduling (ICAPS'10), 2010.

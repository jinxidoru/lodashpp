
CXXFLAGS += -std=c++1y


check: lodash_tests
	@echo; echo ' --== running tests ==--'
	@./lodash_tests

lodash_tests: src/lodash_tests.cpp src/lodashpp.hpp
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f ./lodash_tests

watch:
	@while [ true ]; do \
		SHAX=`shasum src/*`; \
		if [ x"$$SHAX" != x"$$SHAY" ]; then \
			make check; \
			SHAY="$$SHAX"; \
		fi; \
		sleep 1; \
	done



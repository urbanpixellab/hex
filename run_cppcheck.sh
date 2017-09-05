cppcheck --check-config \
--enable=all \
--suppress=missingIncludeSystem \
-Isrc \
src

cppcheck \
--enable=all \
--suppress=missingIncludeSystem \
-Isrc \
src

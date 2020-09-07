# Lime
A relational data library for achieving good performance of large data sets with little scaffolding or explicit dependency management. The resulting code is intended to look roughly like single-threaded code. This library is not expected to reach 1.0 for a while. The goals of this library may also cause frequent major version bumps with many breakages in backward compatibility (ex. code which was executed on the main thread by default may not be after scheduler changes).

This is not ready for use. Ultimately its intended to gain more features over time as I need them.

### Known feature list
- [x] Multi-table database with support for various column arrangements
- [x] Automatic type identification
- [x] Can iterate each row of all tables with specific columns
- [x] Can retreive the total number of tables or rows associated with a query
- [x] Lambda support
- [ ] Can move a row from one table to another
- [ ] Can remove rows
- [ ] Cleans up memory
- [ ] Multi-threading
- [ ] Complex data types in cells
- [ ] Type combinator aliases. (ex. columns PositionX, PositionY, PositionZ could be accessed by a special PostionXY type which joins X and Y)
- [ ] Optimization for 0-size cells
- [ ] Optimization for whole tables to use same cell values
- [ ] Partition/sort results by cell values. This can reduce context switching.
- [ ] Proper support for data values which are used by tasks but not stored in database, including dependency scheduler.
- [ ] Global-to-database values
- [ ] Access requirements for various types of data (any thread, main thread, one thread, etc.)
- [ ] Access requirements for various types of tasks (any thread, main thread, one thread, etc.)
- [ ] Blocking tasks which wait for all dependencies and the relevant task to finish
- [ ] Generalize behavior which does not need to be inlined or templated
- [ ] Databases are fully (cheaply) copyable. Useful for performing client-side prediction using copies.
- [ ] Ability to set implicitly ignored types. Useful for a "marked for deletion" tag.
- [ ] Memory efficient
- [ ] Cross-platform

### Installation
Works fine with conan, but it is not published yet.
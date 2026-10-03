

/*
 * Emscripten SjLj compatibility shim for legacy prebuilt libraries ( m103)
 */

#include <stdint.h>
#include <stdlib.h>

static uintptr_t setjmpId = 0;

typedef struct TableEntry {
  uintptr_t id;
  uint32_t label;
} TableEntry;

extern void setTempRet0(uint32_t value);

TableEntry* saveSetjmp(uintptr_t* env, uint32_t label, TableEntry* table, uint32_t size) {
  uint32_t i = 0;
  setjmpId++;
  *env = setjmpId;
  while (i < size) {
    if (table[i].id == 0) {
      table[i].id = setjmpId;
      table[i].label = label;
      // prepare next slot
      table[i + 1].id = 0;
      setTempRet0(size);
      return table;
    }
    i++;
  }
  // grow the table
  size *= 2;
  table = (TableEntry*)realloc(table, sizeof(TableEntry) * (size + 1));
  table = saveSetjmp(env, label, table, size);
  setTempRet0(size);
  return table;
}

uint32_t testSetjmp(uintptr_t id, TableEntry* table, uint32_t size) {
  uint32_t i = 0;
  while (i < size) {
    uintptr_t curr = table[i].id;
    if (curr == 0) break;
    if (curr == id) {
      return table[i].label;
    }
    i++;
  }
  return 0;
}

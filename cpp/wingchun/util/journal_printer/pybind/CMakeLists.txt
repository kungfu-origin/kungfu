PROJECT(pybind11)

PYBIND11_ADD_MODULE(pyjournalprinter SHARED "journal_printer_ext.cpp")
TARGET_LINK_LIBRARIES(pyjournalprinter PRIVATE journal_printer ${Boost_LIBRARIES})

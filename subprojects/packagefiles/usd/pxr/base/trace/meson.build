cpp = meson.get_compiler('cpp')

trace_lib = shared_library(
    'usd_trace',
    [
        # PUBLIC_CLASSES
        'aggregateTree.cpp',
        'aggregateNode.cpp',
        'category.cpp',
        'collection.cpp',
        'collectionNotice.cpp',
        'collector.cpp',
        'counterAccumulator.cpp',
        'dataBuffer.cpp',
        'dynamicKey.cpp',
        'event.cpp',
        'eventContainer.cpp',
        'eventData.cpp',
        'eventList.cpp',
        'eventNode.cpp',
        'eventTree.cpp',
        'key.cpp',
        'reporter.cpp',
        'reporterBase.cpp',
        'reporterDataSourceBase.cpp',
        'reporterDataSourceCollection.cpp',
        'reporterDataSourceCollector.cpp',
        'serialization.cpp',
        'staticKeyData.cpp',
        'threads.cpp',

        # PRIVATE_CLASSES
        'aggregateTreeBuilder.cpp',
        'eventTreeBuilder.cpp',
        'jsonSerialization.cpp',

    ],
    install: true,
    implicit_include_directories: false,
    include_directories: pxr_inc,
    cpp_args: args + ['-DTRACE_EXPORTS=1'],
    dependencies: [boost_dep, tbb_dep],
    link_with: [arch_lib, js_lib, tf_lib],
)

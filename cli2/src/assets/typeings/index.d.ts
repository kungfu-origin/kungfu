interface ProcessListItem {
    processId: string,
    processName: string,
    type: string,
    typeName: string,
    status: string,
    [propName: string]: any
}


interface logDataWithProcessId {
    updateTime: string,
    message: string,
    processId: string 
}
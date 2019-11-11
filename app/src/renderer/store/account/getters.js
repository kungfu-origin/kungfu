
export const getSourceList = (state) => {
    return state.accountList.filter(a => !!a.receive_md)
}

export const getSourceListOptions = (state) => {
    return state.accountList
        .filter(a => !!a.receive_md)
        .map(a => ({ value: a.source_name, name: a.source_name }))

}
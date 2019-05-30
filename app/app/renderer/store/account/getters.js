
export const getSourceList = (state) => {
    return state.accountList.filter(a => (a.receive_md == true))
}
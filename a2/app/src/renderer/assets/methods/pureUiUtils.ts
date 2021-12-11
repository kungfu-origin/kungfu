export const removeLoadingMask = (): void => {
    const $loadingMask = document.getElementById('loading');
    if ($loadingMask) $loadingMask.remove();
};

export const parseURIParams = (): Record<string, string> => {
    const search = window.location.search;
    const searchResolved = search.slice(search.indexOf('?') + 1);
    const searchResolvedSplits = searchResolved.split('&');
    const paramsData: Record<string, string> = {};
    searchResolvedSplits.forEach((item: string) => {
        const itemSplit = item.split('=');
        if (itemSplit.length === 2) {
            paramsData[itemSplit[0] || ''] = itemSplit[1] || '';
        }
    });

    return paramsData;
};

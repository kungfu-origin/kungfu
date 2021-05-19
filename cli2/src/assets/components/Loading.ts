const blessed = require('blessed');
export default (screen: any) => {
    return blessed.loading({
        parent: screen,
        top: '100%-5',
        left: '100%-40',
        height: 5,
        align: 'left',
        valign: 'center',
        width: 40,
        tags: true,
        hidden: true,
        border: 'line'
    });
}
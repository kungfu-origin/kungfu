const blessed = require('blessed');
export default (screen: any) => {
    return blessed.message({
        parent: screen,
        top: '100%-5',
        left: '100%-40',
        height: 5,
        padding: {
            left: 1,
            right: 1,
            top: 0,
            bottom: 0
        },			
        align: 'left',
        valign: 'center',
        width: 40,
        tags: true,
        hidden: true,
        border: 'line',
        style: {
            bg: 'blue'
        }
    });
}
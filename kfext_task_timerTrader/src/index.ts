

process.on('message', (packet) => {
    console.log([packet, '---'])
})

let count = 0

setInterval(() => {
    console.log('send', count++)
    //@ts-ignore
    process.send({
        type: 'process:msg',
        data: {
            success: true,
            data: {
                aaa: 111
            }
        }
    })
}, 10000)


process.on('message', (packet) => {
    console.log([packet, '---'])
})

setInterval(() => {
    console.log('send')
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
}, 1000)
import axios from "axios";
// @ts-ignore
import httpAdapters from 'axios/lib/adapters/http';

axios.defaults.adapter = httpAdapters;

export const testWorkable = () => {
    const token = getToken();
    return axios.get("https://8797d8rmi8.execute-api.cn-north-1.amazonaws.com.cn/testWorkable", {
        headers: {
            "Authorization": `Bearer ${token}`
        }
    })    
    .then(res => {
        console.log(res.data)
        return res.data
    })
    .catch(err => {
        console.log(err.message)
        console.log(err.response)
    })
}


export const getKungfuExts = () => {
    const token = getToken();
    return axios.get("https://eeda505r8b.execute-api.cn-north-1.amazonaws.com.cn/listKungfuExts", {
        headers: {
            "Authorization": `Bearer ${token}`
        }
    })    
    .then((res: any) => {
        console.log(res.data)
        return res.data;
    })
    .catch((err: Error) => {
        console.log(err.message)
    })
}


function getToken () {
    return localStorage.getItem("login_token") || ""
}
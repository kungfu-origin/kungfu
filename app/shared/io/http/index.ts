import axios, { AxiosInstance, AxiosPromise } from "axios";
//@ts-ignore
import httpAdapters from 'axios/lib/adapters/http';

axios.defaults.adapter = httpAdapters;

export const getS3Pool = () => {
    const token = getToken();
    return axios({
        method: 'get',
        url: "https://6czfnb6y0g.execute-api.cn-north-1.amazonaws.com.cn/test-lambda",
        headers: {
            "Authorization": `Bearer ${token}`,
        }
    })
    .then(res => {
        return res.data
    })
}

getS3Pool()

function getToken () {
    return localStorage.getItem("_authing_token") || ""
}
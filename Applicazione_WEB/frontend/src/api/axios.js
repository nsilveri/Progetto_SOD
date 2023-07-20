import axios from 'axios';
const BASE_URL = process.env.REACT_APP_HOST;

export default axios.create({
    baseURL: BASE_URL
})

import axios from 'axios';

const apiCli = axios.create({

    baseURL: 'https://vj6ftrfp-8000.euw.devtunnels.ms',

    headers: {
        'X-Tunnel-Skip-AntiPhishing-Page': 'true',
        'Accept': 'application/json'
    },

    timeout: 5000

});

export default apiCli;
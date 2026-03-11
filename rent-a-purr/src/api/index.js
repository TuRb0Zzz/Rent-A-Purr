// // Имитация задержки сети
// const delay = (ms) => new Promise(resolve => setTimeout(resolve, ms));
//
// // --- МОКОВЫЕ ДАННЫЕ БЕКЕНДА (База данных в памяти браузера) ---
// let mockUsers =[{ id: 123, username: '123', password: '123', nickname: 'Иван Петров', email: 'user@example.com' }];
// let mockCats =[
//     {
//         id: 1, name: "Барсик", description: "Крупный, пушистый и очень ласковый кот.", breed: "Мейн-кун", age: "2 года",
//         filename:["https://images.unsplash.com/photo-1513360371669-4adf3dd7dff8?auto=format&fit=crop&q=80&w=800"],
//         tags: ["Крупный", "Ласковый", "Любит спать"],
//         bookings: [[1, "2026-03-15 10:00:00", "2026-03-15 18:00:00"]],
//         medical:[
//             { id: 1, icon: "syringe", label: "Привит", color: "#FFFFFF", bg: "#FF2B4A" },
//             { id: 2, icon: "heartPulse", label: "Здоров", color: "#FFFFFF", bg: "#00D26A" }
//         ]
//     },
//     {
//         id: 2, name: "Снежок", description: "Активный и очень любознательный малыш.", breed: "Турецкая ангора", age: "1 год",
//         filename:["https://images.unsplash.com/photo-1533738363-b7f9aef128ce?auto=format&fit=crop&q=80&w=800"],
//         tags:["Энергичный", "Любопытный", "К лотку приучен"],
//         bookings: [],
//         medical:[{ id: 1, icon: "syringe", label: "Привит", color: "#FFFFFF", bg: "#FF2B4A" }]
//     }
// ];
// let mockBookings =[
//     { id: 1, cat_id: 1, user_id: 123, start_time: "2026-03-15 10:00:00", end_time: "2026-03-15 18:00:00", status: 1 } // status 1 = confirmed, 0 = pending
// ];
//
// // Текущая "сессия"
// let currentUser = null;
//
// // --- API ИНТЕРФЕЙС ---
//
// export const api = {
//     // --- AUTH ---
//     register: async (data) => {
//         await delay(800);
//         if (!data.username || !data.password || !data.nickname) throw new Error("Bad Json");
//         if (mockUsers.find(u => u.username === data.username)) throw new Error("User already exists");
//
//         const newUser = { id: Date.now(), ...data, email: data.username + '@mail.com' };
//         mockUsers.push(newUser);
//         currentUser = newUser; // Авто-логин после реги
//         return { status: "ok", message: "User created", nickname: newUser.nickname };
//     },
//
//     login: async (data) => {
//         await delay(500);
//         // В реальности это GET запрос, но параметры передаются либо в URL, либо в заголовках.
//         const user = mockUsers.find(u => u.username === data.username && u.password === data.password);
//         if (!user) throw new Error("Wrong username or password");
//         currentUser = user;
//         return { status: "ok", message: "User authorized", nickname: user.nickname };
//     },
//
//     logout: async () => {
//         await delay(300);
//         currentUser = null;
//     },
//
//     getProfile: async () => {
//         await delay(500);
//         if (!currentUser) throw new Error("Unauthorized");
//         const userBookings = mockBookings.filter(b => b.user_id === currentUser.id);
//         return {
//             status: "ok",
//             user: { user_id: currentUser.id, nickname: currentUser.nickname, email: currentUser.email, bookings: userBookings }
//         };
//     },
//
//     // --- CATS ---
//     getCats: async () => {
//         await delay(600);
//         return { status: "ok", cats: [...mockCats] };
//     },
//
//     addCat: async (formData) => {
//         await delay(1000);
//         if (!currentUser) throw new Error("Unauthorized");
//
//         // В реальности вы отправляете FormData, здесь мы имитируем распаковку
//         const newCat = {
//             id: Date.now(),
//             name: formData.get('name'),
//             description: formData.get('description'),
//             breed: formData.get('breed'),
//             age: formData.get('age'),
//             filename:[formData.get('image') || "https://images.unsplash.com/photo-1514888286974-6c03e2ca1dba?auto=format&fit=crop&q=80&w=800"],
//             tags: formData.get('tags') ? formData.get('tags').split(',') : [],
//             bookings: [],
//             medical:[]
//         };
//
//         const medicalRaw = formData.get('medical');
//         if (medicalRaw) {
//             const parts = medicalRaw.split(',');
//             if (parts.length % 4 !== 0) throw new Error("Medical fields must be multiple of 4");
//             for(let i=0; i < parts.length; i+=4) {
//                 newCat.medical.push({ id: Date.now()+i, icon: parts[i], label: parts[i+1], color: parts[i+2], bg: parts[i+3] });
//             }
//         }
//
//         mockCats.push(newCat);
//         return { status: "ok", ...newCat };
//     },
//
//     updateCat: async (id, data) => {
//         await delay(800);
//         if (!currentUser) throw new Error("Unauthorized");
//         const catIndex = mockCats.findIndex(c => c.id === id);
//         if (catIndex === -1) throw new Error("Cat not found");
//
//         if (data.tags) mockCats[catIndex].tags = data.tags.split(',');
//         if (data.medical) {
//             const parts = data.medical.split(',');
//             const newMed =[];
//             for(let i=0; i < parts.length; i+=4) newMed.push({ id: Date.now()+i, icon: parts[i], label: parts[i+1], color: parts[i+2], bg: parts[i+3] });
//             mockCats[catIndex].medical = newMed;
//         }
//         return { status: "ok", cat_id: id, tags: mockCats[catIndex].tags, medical: mockCats[catIndex].medical };
//     },
//
//     // --- BOOKINGS (USER) ---
//     addBooking: async (data) => {
//         await delay(800);
//         if (!currentUser) throw new Error("Unauthorized");
//         const cat = mockCats.find(c => c.id === data.id);
//         if (!cat) throw new Error("Cat not found");
//
//         const newBooking = { id: Date.now(), cat_id: data.id, user_id: currentUser.id, start_time: data.start, end_time: data.end, status: 0 };
//         mockBookings.push(newBooking);
//         cat.bookings.push([newBooking.id, data.start, data.end]);
//
//         return { status: "ok", message: "Booking created successfully", booking: { ...newBooking, user: { username: currentUser.username, nickname: currentUser.nickname } } };
//     },
//
//     // --- BOOKINGS (ADMIN) ---
//     getAdminBookings: async () => {
//         await delay(600);
//         if (!currentUser) throw new Error("Unauthorized"); // В реальности проверяем права админа
//         const detailedBookings = mockBookings.map(b => {
//             const cat = mockCats.find(c => c.id === b.cat_id);
//             const user = mockUsers.find(u => u.id === b.user_id) || { nickname: "Unknown", email: "admin_added@mail.com" };
//             return { ...b, cat_name: cat?.name, nickname: user.nickname, email: user.email };
//         });
//         return { status: "ok", bookings: detailedBookings };
//     },
//
//     addAdminBooking: async (data) => {
//         await delay(800);
//         if (!currentUser) throw new Error("Unauthorized");
//         const newBooking = { id: Date.now(), cat_id: data.cat_id, user_id: 999, start_time: data.start_time, end_time: data.end_time, status: 1 };
//         mockBookings.push(newBooking);
//         const cat = mockCats.find(c => c.id === data.cat_id);
//         if(cat) cat.bookings.push([newBooking.id, data.start_time, data.end_time]);
//         return { status: "ok", message: "Booking created successfully by admin", booking: newBooking };
//     },
//
//     confirmAdminBooking: async (data) => {
//         await delay(500);
//         if (!currentUser) throw new Error("Unauthorized");
//         const b = mockBookings.find(x => x.id === data.booking_id);
//         if(!b) throw new Error("Booking not found");
//         b.status = 1;
//         return { status: "ok", message: "Booking confirmed successfully" };
//     },
//
//     deleteAdminBooking: async (data) => {
//         await delay(500);
//         if (!currentUser) throw new Error("Unauthorized");
//         mockBookings = mockBookings.filter(x => x.id !== data.booking_id);
//         // Удаляем из кота
//         mockCats.forEach(c => { c.bookings = c.bookings.filter(bk => bk[0] !== data.booking_id) });
//         return { status: "ok", message: "Booking rejected and deleted successfully" };
//     }
// };


export const API_BASE_URL = 'https://vj6ftrfp-8000.euw.devtunnels.ms';


export const getImageUrl = (fileData) => {
    if (!fileData) return 'https://via.placeholder.com/600x400?text=Нет+фото';
    const filename = Array.isArray(fileData) ? fileData[0] : fileData;
    if (!filename || typeof filename !== 'string') return 'https://via.placeholder.com/600x400?text=Ошибка+формата';
    if (filename.startsWith('http')) return filename;
    const cleanFilename = filename.startsWith('/') ? filename.slice(1) : filename;
    return `${API_BASE_URL}/${cleanFilename}`;
};


// Универсальная функция для отправки запросов
async function fetchAPI(endpoint, options = {}) {
    const config = {
        ...options,
        credentials: 'include',
    };

    // Если передаем JSON (а не FormData с файлом), автоматически добавляем заголовки
    if (options.body && !(options.body instanceof FormData)) {
        config.headers = {
            'Content-Type': 'application/json',
            ...config.headers,
        };
        // Превращаем JS объект в строку JSON
        if (typeof options.body === 'object') {
            config.body = JSON.stringify(options.body);
        }
    }

    try {
        const response = await fetch(`${API_BASE_URL}${endpoint}`, config);
        const data = await response.json();

        // Обрабатываем ошибки по вашей схеме (4xx, 5xx или status: "bad")
        if (!response.ok || data.status === 'bad') {
            throw new Error(data.message || data.error || 'Произошла ошибка на сервере');
        }

        return data;
    } catch (error) {
        // Перехватываем сетевые ошибки (например, сервер выключен)
        if (error.name === 'TypeError' && error.message === 'Failed to fetch') {
            throw new Error('Нет связи с сервером. Проверьте подключение.');
        }
        throw error;
    }
}

// --- API ИНТЕРФЕЙС ---
export const api = {
    // --- АВТОРИЗАЦИЯ ---
    register: (data) => fetchAPI('/register', {
        method: 'POST',
        body: data // { username, password, nickname }
    }),

    /*
      ВНИМАНИЕ: По стандарту HTTP (и в браузере `fetch`) GET-запросы не могут содержать тело (body).
      Если ваш бекенд ждет JSON для логина, лучше переделайте его на POST /login на стороне Drogon.
      В коде ниже я использую POST. Если вы оставите на бекенде GET, вам придется передавать данные
      в URL: fetchAPI(`/login?username=${data.username}&password=${data.password}`)
    */
    login: (data) => fetchAPI('/login', {
        method: 'POST', // Рекомендую поменять на POST в бекенде
        body: data      // { username, password }
    }),

    // Если на бекенде есть ручка для логаута - добавьте её вызов. Если нет, достаточно стереть cookie.
    logout: async () => {
        // await fetchAPI('/logout', { method: 'POST' });
    },

    getProfile: () => fetchAPI('/profile', {
        method: 'GET'
    }),

    // --- КОТЫ ---
    getCats: () => fetchAPI('/cats', {
        method: 'GET'
    }),

    addCat: (formData) => fetchAPI('/cats', {
        method: 'POST',
        body: formData // Передаем FormData напрямую (браузер сам поставит Content-Type: multipart/form-data)
    }),

    updateCat: (id, data) => fetchAPI(`/cats/${id}`, {
        method: 'PUT',
        body: data // { tags, medical }
    }),

    // --- БРОНИРОВАНИЯ (КЛИЕНТ) ---
    addBooking: (data) => fetchAPI('/bookings', {
        method: 'POST',
        body: data // { id, start, end }
    }),

    // --- БРОНИРОВАНИЯ (АДМИН) ---
    getAdminBookings: () => fetchAPI('/bookings/admin', {
        method: 'GET'
    }),

    addAdminBooking: (data) => fetchAPI('/bookings/admin', {
        method: 'POST',
        body: data // { email, cat_id, start_time, end_time }
    }),

    confirmAdminBooking: (data) => fetchAPI('/bookings/admin', {
        method: 'PUT',
        body: data // { booking_id }
    }),

    deleteAdminBooking: (data) => fetchAPI('/bookings/admin', {
        method: 'DELETE',
        body: data // { booking_id }
    }),
};
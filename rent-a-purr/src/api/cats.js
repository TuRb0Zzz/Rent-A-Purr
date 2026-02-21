import apiCli from './client';

const mockCatsData = {
    "cats": [
        { "description": "Крутой", "filename": "1", "name": "Коржик" },
        { "description": "Умная", "filename": "2", "name": "Карамелька" },
        { "description": "Толстый", "filename": "3", "name": "Компот" }
    ],
    "status": "ok"
};

export const catService = {
    getAllCats: async () => {
        // // MOCK VERSION (Comment)
        // return new Promise((resolve) => {
        //     setTimeout(() => {
        //         resolve(mockCatsData);
        //     }, 600);
        // });

        // AXIOS VERSION (Uncomment)

        try {
          const response = await apiCli.get('/cats');
          return response.data;
        } catch (error) {
          console.error("Error fetching cats:", error.response || error.message);
          throw new Error("Failed to load our furry employees.");
        }

    }
};
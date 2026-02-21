import { useState, useEffect } from 'react'
import { catService } from './api/cats';
import './App.scss'

function App() {
    const [cats, setCats] = useState([]);
    const [isLoading, setIsLoading] = useState(true);
    const [error, setError] = useState(null);

    useEffect(() => {
        const fetchCats = async() => {
            try {

                const data = await catService.getAllCats();

                if (data.status === 'ok') {
                    setCats(data.cats);
                } else {
                    throw new Error('Unknown data format received');
                }
            } catch (err) {
                setError(err.message);
            } finally {
                setIsLoading(false);
            }
        };
            fetchCats();
    }, []);

    if (isLoading) return <div className="loading">Looking for available purr-fessionals...</div>;
    if (error) return <div className="error">Error: {error}</div>;

    return (
        <div className="app-container">
            <header>
                <h1>Кото-номика</h1>
                <p>Rent the best cats for any task</p>
            </header>

            <main>
                <div className="cats-grid">
                    {cats.map((cat, index) => (
                        <div className="cat-card" key={index}>
                            <div className="cat-image-placeholder">
                                Photo {cat.filename}
                            </div>
                            <div className="cat-info">
                                <h2>{cat.name}</h2>
                                <p className="description">{cat.description}</p>
                                <button className="rent-btn">Book {cat.name}</button>
                            </div>
                        </div>
                    ))}
                </div>
            </main>
        </div>
    );

}


export default App;

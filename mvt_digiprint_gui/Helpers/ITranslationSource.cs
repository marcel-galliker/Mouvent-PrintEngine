using System.ComponentModel;
using System.Globalization;
using System.Resources;

namespace RX_DigiPrint.Helpers
{
    /// <summary>
    /// Represents a localization manager that provides convenient access to culture-specific resources at run time.
    /// </summary>
    public interface ITranslationSource : INotifyPropertyChanged
    {
        /// <summary>
        /// Returns the value of the string resource localized.
        /// </summary>
        /// <param name="key">The key of the resource to retrieve.</param>
        /// <returns>The value of the resource localized for the specified culture, or <paramref name="key"/> if the resource name cannot be found.</returns>
        string this[string key] { get; }

        /// <summary>
        /// Gets the string resource with the given name and formatted with the supplied arguments.
        /// </summary>
        /// <param name="name">The name of the string resource.</param>
        /// <param name="arguments">The values to format the string with.</param>
        /// <returns>The formatted string resource as a <see cref="System.String"/>.</returns>
        string this[string name, params object[] arguments] { get; }

        /// <summary>
        /// Gets or sets the current culture.
        /// </summary>
        CultureInfo CurrentCulture { get; set; }

        /// <summary>
        /// Sets the current resource manager.
        /// </summary>
        /// <param name="resourceManager">A resource manager.</param>
        /// <exception cref="ArgumentNullException">resourceManager is <see langword="null"/>.</exception>
        void SetResourceManager(ResourceManager resourceManager);
    }

    public static class StringLocalizerExtensions
    {
        /// <summary>
        /// Gets the string resource with the given name.
        /// </summary>
        /// <param name="stringLocalizer">The <see cref="ITranslationSource"/>.</param>
        /// <param name="name">The name of the string resource.</param>
        /// <returns>The string resource as a <see cref="System.String"/>.</returns>
        public static string GetString(this ITranslationSource stringLocalizer, string name)
        {
            return stringLocalizer[name];
        }

        /// <summary>
        /// Gets the string resource with the given name and formatted with the supplied arguments.
        /// </summary>
        /// <param name="stringLocalizer">The <see cref="ITranslationSource"/>.</param>
        /// <param name="name">The name of the string resource.</param>
        /// <param name="arguments">The values to format the string with.</param>
        /// <returns></returns>
        public static string GetString(this ITranslationSource stringLocalizer, string name, params object[] arguments)
        {
            return stringLocalizer[name, arguments];
        }
    }
}
